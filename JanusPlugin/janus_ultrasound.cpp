/*!
 * Heavily based on Janus Streaming Plugin by Lorenzo Miniero <lorenzo@meetecho.com>
 * see https://janus.conf.meetecho.com/
*/

extern "C" {
#include <janus/debug.h>
#include <janus/apierror.h>
#include <janus/config.h>
#include <janus/mutex.h>
#include <janus/rtp.h>
#include <janus/rtcp.h>
#include <janus/record.h>
#include <janus/utils.h>
#include <janus/plugin.h>
}

#include <errno.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <boost/format.hpp>
#include <USPipelineInterface/UltrasoundImagePipeline.h>

#include <json.hpp>
using json = nlohmann::json;

#include "plugin_hooks.h"
#include "janus_ultrasound.h"
#include "RTPSource.h"
#include "rtp_functions.h"
#include "JanusUltrasoundSessionManager.h"
#include "auth/Authenticator.h"
#include "auth/DummyAuthenticator.h"
#include "auth/SimpleAuthenticator.h"

#include <mutex>
#include <queue>
#include <map>

//TODO: move static functions into a class and make below members

static JanusUltrasoundSessionManager* session_manager;
static Authenticator* authenticator;

int idCounter = 1;

/* Plugin creator */
janus_plugin *create(void) {
    JANUS_LOG(LOG_VERB, "%s created!\n", JANUS_ULTRASOUND_NAME);
    return &janus_ultrasound_plugin;
}

JanusUltrasoundSessionManager::JanusUltrasoundSessionManager(janus_callbacks* gateway)
{
    this->gateway = gateway;
    frame_source = createFrameSource();

    g_atomic_int_set(&initialized, 1);

    authenticator = new SimpleAuthenticator();

    /* Launch the thread that will handle incoming messages */
    handler_thread = new std::thread(&JanusUltrasoundSessionManager::messageHandlerThread, this);
}


JanusUltrasoundSessionManager::~JanusUltrasoundSessionManager() {
    delete frame_source;

    {
        // Remove all sessions
        std::unique_lock<std::mutex> lock(sessions_mutex);
        for (auto entry : sessions) {
            delete entry.second;
        }
        sessions.clear();
    }

    if (handler_thread->joinable()) {
        handler_thread->join();
        handler_thread = nullptr;
    }

    while (!messages.empty()) {
        delete messages.front();
        messages.pop();
    }

    g_atomic_int_set(&initialized, 0);
    g_atomic_int_set(&stopping, 0);
}


JanusUltrasoundSession::JanusUltrasoundSession(janus_callbacks* gateway, janus_plugin_session* handle)
{
    this->gateway = gateway;
    this->handle = handle;

    this->mountpoint = NULL;	/* This will happen later */
    this->started = FALSE;	/* This will happen later */
    this->destroyed = 0;
    g_atomic_int_set(&this->hangingup, 0);
}

JanusUltrasoundSession::~JanusUltrasoundSession()
{
    delete pipeline;

    if (mountpoint) {
        janus_mutex_lock(&mountpoint->mutex);
        mountpoint->listeners = g_list_remove_all(mountpoint->listeners, this);
        janus_mutex_unlock(&mountpoint->mutex);

        delete mountpoint;
    }
}

janus_plugin_result* JanusUltrasoundSessionManager::onMessage(janus_plugin_session *handle,
        char *transaction, char *message, char *sdp_type, char *sdp)
{
    if (message == NULL || session_manager->getSession(handle)->destroyed)
    {
        return handleMessageError(handle, transaction, message, NULL, sdp_type, sdp, "Empty message");
    }

    json root;
    try {
        root = json::parse(message);
    } catch (std::invalid_argument e) {
        return handleMessageError(handle, transaction, message, root, sdp_type, sdp, "JSON Error");
    }

    try {
        std::string request = root["request"];
    } catch (std::domain_error e) {
        return handleMessageError(handle, transaction, message, root, sdp_type, sdp, "Missing element (request)");
    }

    addMessageToQueue(session_manager->getSession(handle), transaction, root, sdp_type, sdp);

    return janus_plugin_result_new(JANUS_PLUGIN_OK_WAIT, NULL);
}


void JanusUltrasoundSessionManager::addMessageToQueue(JanusUltrasoundSession* session,
    char *transaction, json root, char *sdp_type, char *sdp)
{
    {
        std::unique_lock<std::mutex> lock(messages_mutex);
        messages.push(new Message(session, transaction, root, sdp, sdp_type));
    }
}


/* Thread to handle incoming messages */
void JanusUltrasoundSessionManager::messageHandlerThread() {

    while(g_atomic_int_get(&initialized) && !g_atomic_int_get(&stopping)) {

        Message *msg;
        {
            std::unique_lock<std::mutex> lock(messages_mutex);
            if (messages.empty()) continue;
            msg = messages.front();
            messages.pop();
        }

        // Ignore empty messages
        if (msg->message == NULL) {
            continue;
        }

        if(msg->message["request"] == "watch") {
            msg->session->handleMessageWatch(msg);
        } else if(msg->message["request"] == "ready") {
            msg->session->handleMessageReady(msg);
        } else if(msg->message["request"] == "start") {
            msg->session->handleMessageStart(msg);
        } else if(msg->message["request"] == "stop") {
            msg->session->handleMessageStop(msg);
        } else {
            json event;
            event["ultrasound"] = "event";
            event["error"] = "Unknown Request";

            std::string event_str = event.dump();

            gateway->push_event(msg->session->handle, &janus_ultrasound_plugin,
                                msg->transaction, event_str.c_str(), NULL, NULL);
            delete msg;
        }
    }
    return NULL;
}


void JanusUltrasoundSession::handleMessageWatch(Message* msg) {

    /* RTP live source (e.g., from gstreamer/ffmpeg/vlc/etc.) */
    char* desc = "ULTRASOUND";
    int pin = 0;
    int vport = msg->session->getPort();
    int vcodec = 100;
    char* vrtpmap = "VP8/90000";
    gboolean is_private = false;

    if (!authenticator->isValid(msg->message["auth"])) {
        json event;
        event["ultrasound"] = "event";
        event["error"] = "Authentication failed";

        std::string event_str = event.dump();
        gateway->push_event(msg->session->handle, &janus_ultrasound_plugin, msg->transaction,
                            event_str.c_str(), NULL, NULL);
        delete msg;
        return;
    }

    RTPSource* mp = new RTPSource(idCounter++, desc, vport, vcodec, vrtpmap);

    stopping = FALSE;
    mountpoint = mp;

    /* TODO Check if user is already watching a stream, if the video is active, etc. */
    janus_mutex_lock(&mp->mutex);
    mp->listeners = g_list_append(mp->listeners, this);
    janus_mutex_unlock(&mp->mutex);
    char* sdp_type = "offer";	/* We're always going to do the offer ourselves, never answer */

    char* sdp = mp->createSDP();

    json result;
    result["status"] = "preparing";

    sendPostMessageEvent(result, msg, sdp, sdp_type);
}


void JanusUltrasoundSession::handleMessageStart(Message* msg) {
    json result;
    /* We wait for the setup_media event to start: on the other hand, it may have already arrived */
    result["status"] = started ? "started" : "starting";
    sendPostMessageEvent(result, msg, NULL, NULL);
}


void JanusUltrasoundSession::handleMessageStop(Message *msg) {
    if(stopping || !started) {
        delete msg;
        return;
    }
    JANUS_LOG(LOG_VERB, "Stopping the ultrasound\n");
    stopping = TRUE;
    started = FALSE;

    json result;
    result["status"] = "stopping";
    if(mountpoint) {
        janus_mutex_lock(&mountpoint->mutex);
        JANUS_LOG(LOG_VERB, "  -- Removing the session from the mountpoint listeners\n");
        if(g_list_find(mountpoint->listeners, this) != NULL) {
            JANUS_LOG(LOG_VERB, "  -- -- Found!\n");
        }
        mountpoint->listeners = g_list_remove_all(mountpoint->listeners, this);
        janus_mutex_unlock(&mountpoint->mutex);
    }
    mountpoint = NULL;
    /* Tell the core to tear down the PeerConnection, hangup_media will do the rest */
    gateway->close_pc(handle);
    sendPostMessageEvent(result, msg, NULL, NULL);
}

void JanusUltrasoundSession::handleMessageReady(Message* msg) {
    start();
}

janus_plugin_result* JanusUltrasoundSessionManager::handleMessageError(janus_plugin_session *handle,
    char *transaction, char* message, json root, char *sdp_type, char *sdp, char* error)
{
    free(transaction);
    free(message);
    free(sdp_type);
    free(sdp);

    /* Prepare JSON error event */
    json event;
    event["ultrasound"] = "event";
    event["error"] = error;

    std::string event_str = event.dump();
    return janus_plugin_result_new(JANUS_PLUGIN_OK, event_str.c_str());
}

void JanusUltrasoundSession::sendPostMessageEvent(json result, Message* msg, char* sdp, char* sdp_type)
{
    /* Prepare JSON event */
    json event;
    event["ultrasound"] = "event";
    if(result != NULL)
        event["result"] = result;

    std::string event_str = event.dump();
    gateway->push_event(handle, &janus_ultrasound_plugin, msg->transaction,
                        event_str.c_str(), sdp_type, sdp);

    if (sdp) free(sdp);
    delete msg;
}

void JanusUltrasoundSessionManager::onDataReceived(janus_plugin_session *handle, char* buffer, int length) {
    // Check session health
    JanusUltrasoundSession* session = session_manager->getSession(handle);

    // Check message
    if (buffer == NULL || length <= 0) {
        return;
    }

    // Pass on null-terminated string
    char* msg = (char*) malloc(length+1);
    memcpy(msg, buffer, length);
    msg[length] = '\0';

    sessions[handle]->onDataReceived(msg);

    free(msg);
}




Message::Message(JanusUltrasoundSession* session, char *transaction, json message, char *sdp, char *sdp_type) {
    this->session = session;
    this->transaction = transaction;
    this->message = message;
    this->sdp = sdp;
    this->sdp_type = sdp_type;
}

Message::~Message() {
    if (transaction) free(transaction);
    if (sdp_type) free(sdp_type);
    if (sdp) free(sdp);
}



/* C Function Wrappers */

void janus_ultrasound_incoming_data(janus_plugin_session *handle, char* buffer, int length)
{
    // Check plugin health
    if (handle == NULL || handle->stopped || g_atomic_int_get(&stopping) ||
        !g_atomic_int_get(&initialized) || gateway == NULL)
    {
        return;
    }

    session_manager->onDataReceived(handle, buffer, length);
}

janus_plugin_result* janus_ultrasound_handle_message(janus_plugin_session *handle, char *transaction,
                                                     char *message, char *sdp_type, char *sdp)
{

    if (g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized)) {
        return janus_plugin_result_new(JANUS_PLUGIN_ERROR,
               g_atomic_int_get(&stopping) ? "Shutting down" : "Plugin not initialized");
    }

    return session_manager->onMessage(handle, transaction, message, sdp_type, sdp);
}


void janus_ultrasound_setup_media(janus_plugin_session *handle) {

    if (g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized))
        return;
    JanusUltrasoundSession* session = session_manager->getSession(handle);
    if (!session || session->destroyed) {
        return;
    }

    g_atomic_int_set(&session->hangingup, 0);
    /* We only start ultrasound towards this user when we get this event */
    session->context.v_last_ssrc = 0;
    session->context.v_last_ts = 0;
    session->context.v_base_ts = 0;
    session->context.v_base_ts_prev = 0;
    session->context.v_last_seq = 0;
    session->context.v_base_seq = 0;
    session->context.v_base_seq_prev = 0;

    session->started = TRUE;
    /* Prepare JSON event */
    json event;
    event["ultrasound"] = "event";
    event["result"]["status"] = "started";

    std::string event_str = event.dump();
    gateway->push_event(handle, &janus_ultrasound_plugin, NULL, event_str.c_str(), NULL, NULL);
}


void janus_ultrasound_hangup_media(janus_plugin_session *handle) {
    if (g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized))
        return;
    JanusUltrasoundSession* session = session_manager->getSession(handle);
    if (!session || session->destroyed || g_atomic_int_add(&session->hangingup, 1)) {
        JANUS_LOG(LOG_ERR, "No session associated with this handle...\n");
        return;
    }

    /* Simulate a "stop" coming from the browser */
    session_manager->addMessageToQueue(session, NULL, json::parse("{\"request\":\"stop\"}"), NULL, NULL);
}


void janus_ultrasound_incoming_rtp(janus_plugin_session *handle, int video, char *buf, int len) {
    if (handle == NULL || handle->stopped || g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized))
        return;
    /* FIXME We don't care about what the browser sends us, we're sendonly */
}

void janus_ultrasound_incoming_rtcp(janus_plugin_session *handle, int video, char *buf, int len) {
    if (handle == NULL || handle->stopped || g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized))
        return;
    /* We might interested in the available bandwidth that the user advertizes */
    uint64_t bw = janus_rtcp_get_remb(buf, len);
    if(bw > 0) {
        JANUS_LOG(LOG_HUGE, "REMB for this PeerConnection: %"SCNu64"\n", bw);
        /* TODO Use this somehow (e.g., notification towards application?) */
    }
    /* FIXME Maybe we should care about RTCP, but not now */
}

void janus_ultrasound_create_session(janus_plugin_session *handle, int *error) {
    if(g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized)) {
        *error = -1;
        return;
    }

    JanusUltrasoundSession* session = new JanusUltrasoundSession(gateway, handle);
    session_manager->addSession(session, handle);
}

void janus_ultrasound_destroy_session(janus_plugin_session *handle, int *error) {
    if(g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized)) {
        *error = -1;
        return;
    }

    session_manager->destroySession(handle);
}

char *janus_ultrasound_query_session(janus_plugin_session *handle) {
    if(g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized)) {
        return NULL;
    }

    JanusUltrasoundSession* session = session_manager->getSession(handle);
    if(!session) {
        JANUS_LOG(LOG_ERR, "No session associated with this handle...\n");
        return NULL;
    }

    /* What is this user watching, if anything? */
    json info;
    info["state"] = session->mountpoint ? "watching" : "idle";
    if(session->mountpoint) {
        info["mountpoint_id"] = session->mountpoint->id;
        info["mountpoint_name"] = session->mountpoint->name ? session->mountpoint->name : nullptr;
    }
    info["destroyed"] = session->destroyed;
    std::string info_text = info.dump(info);
    return info_text.c_str();
}


/* Plugin implementation */
int janus_ultrasound_init(janus_callbacks *callback, const char *config_path) {
    if(g_atomic_int_get(&stopping)) {
        return -1;
    }

    gateway = callback;
    session_manager = new JanusUltrasoundSessionManager(gateway);

    return 0;
}

void janus_ultrasound_destroy(void) {
    if(!g_atomic_int_get(&initialized))
        return;
    g_atomic_int_set(&stopping, 1);

    delete session_manager;

    JANUS_LOG(LOG_INFO, "%s destroyed!\n", JANUS_ULTRASOUND_NAME);
}
