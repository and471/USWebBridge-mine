/*!
 * Heavily based on Janus Streaming Plugin by Lorenzo Miniero <lorenzo@meetecho.com>
 * see https://janus.conf.meetecho.com/
*/

#include <jansson.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <boost/format.hpp>

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

#include "plugin_hooks.h"
#include "janus_ultrasound.h"
#include "RTPSource.h"

#include <USPipelineInterface/UltrasoundImagePipeline.h>
#include "JanusUltrasoundPlugin.h"


/* Plugin creator */
janus_plugin *create(void) {
    JANUS_LOG(LOG_VERB, "%s created!\n", JANUS_ULTRASOUND_NAME);
    return &janus_ultrasound_plugin;
}

static JanusUltrasoundPlugin* us_plugin;

/* Useful stuff */
static volatile gint initialized = 0, stopping = 0;
static janus_callbacks *gateway = NULL;
static GThread *handler_thread;
static void *janus_ultrasound_handler(void *data);
static void janus_ultrasound_relay_rtp_packet(gpointer data, gpointer user_data);
static void *janus_ultrasound_relay_thread(void *data);


GHashTable *mountpoints;
static GList *old_mountpoints;
janus_mutex mountpoints_mutex;

/* Helper to create an RTP live source (e.g., from gstreamer/ffmpeg/vlc/etc.) */
RTPSource* janus_ultrasound_create_rtp_source(uint64_t id, char *name, uint16_t vport,
                                              uint8_t vcodec, char *vrtpmap);

static GAsyncQueue *messages = NULL;
static janus_ultrasound_message exit_message;

static void janus_ultrasound_message_free(janus_ultrasound_message *msg) {
	if(!msg || msg == &exit_message)
		return;

	msg->handle = NULL;

	g_free(msg->transaction);
	msg->transaction = NULL;
	if(msg->message)
		json_decref(msg->message);
	msg->message = NULL;
	g_free(msg->sdp_type);
	msg->sdp_type = NULL;
	g_free(msg->sdp);
	msg->sdp = NULL;

	g_free(msg);
}


static GHashTable *sessions;
static GList *old_sessions;
static janus_mutex sessions_mutex;

/* Packets we get from gstreamer and relay */
typedef struct janus_ultrasound_rtp_relay_packet {
	rtp_header *data;
    gint length;
	uint32_t timestamp;
	uint16_t seq_number;
} janus_ultrasound_rtp_relay_packet;


/* Error codes */
#define JANUS_ULTRASOUND_ERROR_NO_MESSAGE			450
#define JANUS_ULTRASOUND_ERROR_INVALID_JSON			451
#define JANUS_ULTRASOUND_ERROR_INVALID_REQUEST		452
#define JANUS_ULTRASOUND_ERROR_MISSING_ELEMENT		453
#define JANUS_ULTRASOUND_ERROR_INVALID_ELEMENT		454
#define JANUS_ULTRASOUND_ERROR_NO_SUCH_MOUNTPOINT	455
#define JANUS_ULTRASOUND_ERROR_CANT_CREATE			456
#define JANUS_ULTRASOUND_ERROR_UNAUTHORIZED			457
#define JANUS_ULTRASOUND_ERROR_CANT_SWITCH			458
#define JANUS_ULTRASOUND_ERROR_UNKNOWN_ERROR		470




/* Plugin implementation */
int janus_ultrasound_init(janus_callbacks *callback, const char *config_path) {
    if(g_atomic_int_get(&stopping)) {
        /* Still stopping from before */
        return -1;
    }

    mountpoints = g_hash_table_new(NULL, NULL);
    janus_mutex_init(&mountpoints_mutex);

    sessions = g_hash_table_new(NULL, NULL);
    janus_mutex_init(&sessions_mutex);
    messages = g_async_queue_new_full((GDestroyNotify) janus_ultrasound_message_free);
    /* This is the callback we'll need to invoke to contact the gateway */
    gateway = callback;
    g_atomic_int_set(&initialized, 1);


    GError *error = NULL;
    /* Launch the thread that will handle incoming messages */
    handler_thread = g_thread_try_new("janus ultrasound handler", janus_ultrasound_handler, NULL, &error);
    if(error != NULL) {
        g_atomic_int_set(&initialized, 0);
        JANUS_LOG(LOG_ERR, "Got error %d (%s) trying to launch the Streaming handler thread...\n", error->code, error->message ? error->message : "??");
        return -1;
    }

    us_plugin = new JanusUltrasoundPlugin(gateway);

    JANUS_LOG(LOG_INFO, "%s initialized!\n", JANUS_ULTRASOUND_NAME);

    return 0;
}

void janus_ultrasound_destroy(void) {
	if(!g_atomic_int_get(&initialized))
		return;
	g_atomic_int_set(&stopping, 1);

	g_async_queue_push(messages, &exit_message);

	if(handler_thread != NULL) {
		g_thread_join(handler_thread);
		handler_thread = NULL;
	}

	/* Remove all mountpoints */
	janus_mutex_unlock(&mountpoints_mutex);
	GHashTableIter iter;
	gpointer value;
	g_hash_table_iter_init(&iter, mountpoints);
	while (g_hash_table_iter_next(&iter, NULL, &value)) {
        RTPSource* mp = value;
        delete mp;
	}
	janus_mutex_unlock(&mountpoints_mutex);

	/* FIXME We should destroy the sessions cleanly */
	usleep(500000);
	janus_mutex_lock(&mountpoints_mutex);
	g_hash_table_destroy(mountpoints);
	janus_mutex_unlock(&mountpoints_mutex);
	janus_mutex_lock(&sessions_mutex);
	g_hash_table_destroy(sessions);
	janus_mutex_unlock(&sessions_mutex);
	g_async_queue_unref(messages);
	messages = NULL;
	sessions = NULL;


	g_atomic_int_set(&initialized, 0);
	g_atomic_int_set(&stopping, 0);

    delete us_plugin;

	JANUS_LOG(LOG_INFO, "%s destroyed!\n", JANUS_ULTRASOUND_NAME);
}

void janus_ultrasound_create_session(janus_plugin_session *handle, int *error) {

	if(g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized)) {
		*error = -1;
		return;
	}	
	janus_ultrasound_session *session = (janus_ultrasound_session *)g_malloc0(sizeof(janus_ultrasound_session));
	if(session == NULL) {
		JANUS_LOG(LOG_FATAL, "Memory error!\n");
		*error = -2;
		return;
	}
	session->handle = handle;
	session->mountpoint = NULL;	/* This will happen later */
    session->started = FALSE;	/* This will happen later */
	session->destroyed = 0;
	g_atomic_int_set(&session->hangingup, 0);
	handle->plugin_handle = session;
	janus_mutex_lock(&sessions_mutex);
	g_hash_table_insert(sessions, handle, session);
    janus_mutex_unlock(&sessions_mutex);

    us_plugin->newSession(handle);

	return;
}

void janus_ultrasound_destroy_session(janus_plugin_session *handle, int *error) {

	if(g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized)) {
		*error = -1;
		return;
	}	

    us_plugin->destroySession(handle);

	janus_ultrasound_session *session = (janus_ultrasound_session *)handle->plugin_handle; 
	if(!session) {
		JANUS_LOG(LOG_ERR, "No session associated with this handle...\n");
		*error = -2;
		return;
	}
	JANUS_LOG(LOG_VERB, "Removing ultrasound session...\n");
	if(session->mountpoint) {

        printf("REMOVING MOUNTPOINT\n");
		janus_mutex_lock(&session->mountpoint->mutex);
        session->mountpoint->listeners = g_list_remove_all(session->mountpoint->listeners, session);
        janus_mutex_unlock(&session->mountpoint->mutex);

        delete session->mountpoint;
	}
	janus_mutex_lock(&sessions_mutex);
	if(!session->destroyed) {

        printf("Freeing old Streaming session\n");
        session->handle = NULL;
        g_free(session);
		g_hash_table_remove(sessions, handle);
	}
	janus_mutex_unlock(&sessions_mutex);

	return;
}

char *janus_ultrasound_query_session(janus_plugin_session *handle) {
	if(g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized)) {
		return NULL;
	}	
	janus_ultrasound_session *session = (janus_ultrasound_session *)handle->plugin_handle;
	if(!session) {
		JANUS_LOG(LOG_ERR, "No session associated with this handle...\n");
		return NULL;
	}
	/* What is this user watching, if anything? */
	json_t *info = json_object();
	json_object_set_new(info, "state", json_string(session->mountpoint ? "watching" : "idle"));
	if(session->mountpoint) {
		json_object_set_new(info, "mountpoint_id", json_integer(session->mountpoint->id));
		json_object_set_new(info, "mountpoint_name", session->mountpoint->name ? json_string(session->mountpoint->name) : NULL);
	}
	json_object_set_new(info, "destroyed", json_integer(session->destroyed));
	char *info_text = json_dumps(info, JSON_INDENT(3) | JSON_PRESERVE_ORDER);
	json_decref(info);
	return info_text;
}


/* Thread to handle incoming messages */
static void *janus_ultrasound_handler(void *data) {
    JANUS_LOG(LOG_VERB, "Joining Streaming handler thread\n");
    janus_ultrasound_message *msg = NULL;
    int error_code = 0;
    char *error_cause = g_malloc0(1024);
    if(error_cause == NULL) {
        JANUS_LOG(LOG_FATAL, "Memory error!\n");
        return NULL;
    }
    json_t *root = NULL;
    while(g_atomic_int_get(&initialized) && !g_atomic_int_get(&stopping)) {
        msg = g_async_queue_pop(messages);
        if(msg == NULL)
            continue;
        if(msg == &exit_message)
            break;
        if(msg->handle == NULL) {
            janus_ultrasound_message_free(msg);
            continue;
        }
        janus_ultrasound_session *session = NULL;
        janus_mutex_lock(&sessions_mutex);
        if(g_hash_table_lookup(sessions, msg->handle) != NULL ) {
            session = (janus_ultrasound_session *)msg->handle->plugin_handle;
        }
        janus_mutex_unlock(&sessions_mutex);
        if(!session) {
            JANUS_LOG(LOG_ERR, "No session associated with this handle...\n");
            janus_ultrasound_message_free(msg);
            continue;
        }
        if(session->destroyed) {
            janus_ultrasound_message_free(msg);
            continue;
        }
        /* Handle request */
        error_code = 0;
        root = NULL;
        if(msg->message == NULL) {
            JANUS_LOG(LOG_ERR, "No message??\n");
            error_code = JANUS_ULTRASOUND_ERROR_NO_MESSAGE;
            g_snprintf(error_cause, 512, "%s", "No message??");
            goto error;
        }
        root = msg->message;
        /* Get the request first */
        json_t *request = json_object_get(root, "request");
        if(!request) {
            JANUS_LOG(LOG_ERR, "Missing element (request)\n");
            error_code = JANUS_ULTRASOUND_ERROR_MISSING_ELEMENT;
            g_snprintf(error_cause, 512, "Missing element (request)");
            goto error;
        }
        if(!json_is_string(request)) {
            JANUS_LOG(LOG_ERR, "Invalid element (request should be a string)\n");
            error_code = JANUS_ULTRASOUND_ERROR_INVALID_ELEMENT;
            g_snprintf(error_cause, 512, "Invalid element (request should be a string)");
            goto error;
        }
        const char *request_text = json_string_value(request);
        json_t *result = NULL;
        const char *sdp_type = NULL;
        char *sdp = NULL;
        /* All these requests can only be handled asynchronously */
        if(!strcasecmp(request_text, "watch")) {
            Plugin::handleMessageWatch(session, msg, root);
            continue;
        } else if(!strcasecmp(request_text, "ready")) {
            Plugin::handleMessageReady(session, msg, root);
            continue;
        } else if(!strcasecmp(request_text, "start")) {
            Plugin::handleMessageStart(session, msg, root);
            continue;
        } else if(!strcasecmp(request_text, "stop")) {
            Plugin::handleMessageStop(session, msg, root);
            continue;
        } else {
            JANUS_LOG(LOG_VERB, "Unknown request '%s'\n", request_text);
            error_code = JANUS_ULTRASOUND_ERROR_INVALID_REQUEST;
            g_snprintf(error_cause, 512, "Unknown request '%s'", request_text);
            goto error;
        }

error:
        {
            /* Prepare JSON error event */
            json_t *event = json_object();
            json_object_set_new(event, "ultrasound", json_string("event"));
            json_object_set_new(event, "error_code", json_integer(error_code));
            json_object_set_new(event, "error", json_string(error_cause));
            char *event_text = json_dumps(event, JSON_INDENT(3) | JSON_PRESERVE_ORDER);
            json_decref(event);
            JANUS_LOG(LOG_VERB, "Pushing event: %s\n", event_text);
            int ret = gateway->push_event(msg->handle, &janus_ultrasound_plugin, msg->transaction, event_text, NULL, NULL);
            JANUS_LOG(LOG_VERB, "  >> %d (%s)\n", ret, janus_get_api_error(ret));
            g_free(event_text);
            janus_ultrasound_message_free(msg);
        }
    }
    g_free(error_cause);
    JANUS_LOG(LOG_VERB, "Leaving Streaming handler thread\n");
    return NULL;
}

void Plugin::sendPostMessageEvent(json_t* result, janus_ultrasound_message* msg,
                                  char* sdp, char* sdp_type)
{
    /* Prepare JSON event */
    json_t *event = json_object();
    json_object_set_new(event, "ultrasound", json_string("event"));
    if(result != NULL)
        json_object_set_new(event, "result", result);
    char *event_text = json_dumps(event, JSON_INDENT(3) | JSON_PRESERVE_ORDER);
    json_decref(event);
    JANUS_LOG(LOG_VERB, "Pushing event: %s\n", event_text);
    int ret = gateway->push_event(msg->handle, &janus_ultrasound_plugin, msg->transaction, event_text, sdp_type, sdp);
    JANUS_LOG(LOG_VERB, "  >> %d (%s)\n", ret, janus_get_api_error(ret));
    g_free(event_text);
    if(sdp)
        g_free(sdp);
    janus_ultrasound_message_free(msg);
}

void Plugin::handleMessageStop(janus_ultrasound_session *session, janus_ultrasound_message *msg, json_t *root) {
    if(session->stopping || !session->started) {
        janus_ultrasound_message_free(msg);
        return;
    }
    JANUS_LOG(LOG_VERB, "Stopping the ultrasound\n");
    session->stopping = TRUE;
    session->started = FALSE;
    json_t* result = json_object();
    json_object_set_new(result, "status", json_string("stopping"));
    if(session->mountpoint) {
        janus_mutex_lock(&session->mountpoint->mutex);
        JANUS_LOG(LOG_VERB, "  -- Removing the session from the mountpoint listeners\n");
        if(g_list_find(session->mountpoint->listeners, session) != NULL) {
            JANUS_LOG(LOG_VERB, "  -- -- Found!\n");
        }
        session->mountpoint->listeners = g_list_remove_all(session->mountpoint->listeners, session);
        janus_mutex_unlock(&session->mountpoint->mutex);
    }
    session->mountpoint = NULL;
    /* Tell the core to tear down the PeerConnection, hangup_media will do the rest */
    gateway->close_pc(session->handle);
    Plugin::sendPostMessageEvent(result, msg, NULL, NULL);
}

void Plugin::handleMessageStart(janus_ultrasound_session *session, janus_ultrasound_message* msg, json_t* root) {
    /*if(session->mountpoint == NULL) {
        JANUS_LOG(LOG_VERB, "Can't start: no mountpoint set\n");
        error_code = JANUS_ULTRASOUND_ERROR_NO_SUCH_MOUNTPOINT;
        g_snprintf(error_cause, 512, "Can't start: no mountpoint set");
        goto error;
    }*/
    JANUS_LOG(LOG_VERB, "Starting the ultrasound\n");
    json_t* result = json_object();
    /* We wait for the setup_media event to start: on the other hand, it may have already arrived */
    json_object_set_new(result, "status", json_string(session->started ? "started" : "starting"));
    Plugin::sendPostMessageEvent(result, msg, NULL, NULL);
}

void janus_ultrasound_setup_media(janus_plugin_session *handle) {
	JANUS_LOG(LOG_INFO, "WebRTC media is now available\n");
	if(g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized))
		return;
	janus_ultrasound_session *session = (janus_ultrasound_session *)handle->plugin_handle;	
	if(!session) {
		JANUS_LOG(LOG_ERR, "No session associated with this handle...\n");
		return;
	}
	if(session->destroyed)
		return;
	g_atomic_int_set(&session->hangingup, 0);
	/* We only start ultrasound towards this user when we get this event */
	session->context.v_last_ssrc = 0;
	session->context.v_last_ts = 0;
	session->context.v_base_ts = 0;
	session->context.v_base_ts_prev = 0;
	session->context.v_last_seq = 0;
	session->context.v_base_seq = 0;
	session->context.v_base_seq_prev = 0;
	/* If this is related to a live RTP mountpoint, any keyframe we can shoot already? */
    RTPSource* mountpoint = session->mountpoint;

    session->started = TRUE;
    /* Prepare JSON event */
    json_t *event = json_object();
    json_object_set_new(event, "ultrasound", json_string("event"));
    json_t *result = json_object();
    json_object_set_new(result, "status", json_string("started"));
    json_object_set_new(event, "result", result);
    char *event_text = json_dumps(event, JSON_INDENT(3) | JSON_PRESERVE_ORDER);
    json_decref(event);
    JANUS_LOG(LOG_VERB, "Pushing event: %s\n", event_text);
    int ret = gateway->push_event(handle, &janus_ultrasound_plugin, NULL, event_text, NULL, NULL);
    JANUS_LOG(LOG_VERB, "  >> %d (%s)\n", ret, janus_get_api_error(ret));
    g_free(event_text);

}



void janus_ultrasound_incoming_rtp(janus_plugin_session *handle, int video, char *buf, int len) {
	if(handle == NULL || handle->stopped || g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized))
		return;
	/* FIXME We don't care about what the browser sends us, we're sendonly */
}

void janus_ultrasound_incoming_rtcp(janus_plugin_session *handle, int video, char *buf, int len) {
	if(handle == NULL || handle->stopped || g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized))
		return;
	/* We might interested in the available bandwidth that the user advertizes */
	uint64_t bw = janus_rtcp_get_remb(buf, len);
	if(bw > 0) {
		JANUS_LOG(LOG_HUGE, "REMB for this PeerConnection: %"SCNu64"\n", bw);
		/* TODO Use this somehow (e.g., notification towards application?) */
	}
	/* FIXME Maybe we should care about RTCP, but not now */
}

void janus_ultrasound_hangup_media(janus_plugin_session *handle) {
	JANUS_LOG(LOG_INFO, "No WebRTC media anymore\n");
	if(g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized))
		return;
	janus_ultrasound_session *session = (janus_ultrasound_session *)handle->plugin_handle;	
	if(!session) {
		JANUS_LOG(LOG_ERR, "No session associated with this handle...\n");
		return;
	}
	if(session->destroyed)
		return;
	if(g_atomic_int_add(&session->hangingup, 1))
		return;
	/* FIXME Simulate a "stop" coming from the browser */
	janus_ultrasound_message *msg = g_malloc0(sizeof(janus_ultrasound_message));
	msg->handle = handle;
	msg->message = json_loads("{\"request\":\"stop\"}", 0, NULL);
	msg->transaction = NULL;
	msg->sdp_type = NULL;
	msg->sdp = NULL;
	g_async_queue_push(messages, msg);
}


/* Helpers to create a listener filedescriptor */
static int janus_ultrasound_create_fd(int port, in_addr_t mcast, const char* listenername, const char* medianame, const char* mountpointname) {
	struct sockaddr_in address;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0) {
		JANUS_LOG(LOG_ERR, "[%s] Cannot create socket for %s...\n", mountpointname, medianame);
		return -1;
	}	
	if(port > 0) {
		if(IN_MULTICAST(ntohl(mcast))) {
#ifdef IP_MULTICAST_ALL			
			int mc_all = 0;
			if((setsockopt(fd, IPPROTO_IP, IP_MULTICAST_ALL, (void*) &mc_all, sizeof(mc_all))) < 0) {
				JANUS_LOG(LOG_ERR, "[%s] %s listener setsockopt IP_MULTICAST_ALL failed\n", mountpointname, listenername);
				close(fd);
				return -1;
			}			
#endif			
			struct ip_mreq mreq;
			memset(&mreq, 0, sizeof(mreq));
			mreq.imr_multiaddr.s_addr = mcast;
			if(setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq)) == -1) {
				JANUS_LOG(LOG_ERR, "[%s] %s listener IP_ADD_MEMBERSHIP failed\n", mountpointname, listenername);
				close(fd);
				return -1;
			}
			JANUS_LOG(LOG_ERR, "[%s] %s listener IP_ADD_MEMBERSHIP ok\n", mountpointname, listenername);
		}
	}

	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;
	if(bind(fd, (struct sockaddr *)(&address), sizeof(struct sockaddr)) < 0) {
		JANUS_LOG(LOG_ERR, "[%s] Bind failed for %s (port %d)...\n", mountpointname, medianame, port);
		close(fd);
		return -1;
	}
	return fd;
}


RTPSource* janus_ultrasound_create_rtp_source(uint64_t id, char *name, uint16_t vport,
                                              uint8_t vcodec, char *vrtpmap)
{
	janus_mutex_lock(&mountpoints_mutex);

    int video_fd = janus_ultrasound_create_fd(vport, INADDR_ANY, "Video", "video", name);
    if(video_fd < 0) {
        JANUS_LOG(LOG_ERR, "Can't bind to port %d for video...\n", vport);
        janus_mutex_unlock(&mountpoints_mutex);
        return NULL;
    }

	/* Create the mountpoint */

    RTPSource* rtp_source = new RTPSource(id, strdup(name), vport, video_fd, vcodec, strdup(vrtpmap));

    g_hash_table_insert(mountpoints, GINT_TO_POINTER(rtp_source->id), rtp_source);
    janus_mutex_unlock(&mountpoints_mutex);
    GError *error = NULL;
    g_thread_try_new(rtp_source->name, &janus_ultrasound_relay_thread, rtp_source, &error);

    return rtp_source;
}
		
/* FIXME Test thread to relay RTP frames coming from gstreamer/ffmpeg/others */
static void *janus_ultrasound_relay_thread(void *data) {
	JANUS_LOG(LOG_VERB, "Starting ultrasound relay thread\n");
    RTPSource* mountpoint = (RTPSource*) data;
	if(!mountpoint) {
		JANUS_LOG(LOG_ERR, "Invalid mountpoint!\n");
		g_thread_unref(g_thread_self());
		return NULL;
	}

    int video_fd = mountpoint->video_fd;
	char *name = g_strdup(mountpoint->name ? mountpoint->name : "??");
	/* Needed to fix seq and ts */
    uint32_t v_last_ssrc = 0, v_last_ts = 0, v_base_ts = 0, v_base_ts_prev = 0;
    uint16_t v_last_seq = 0, v_base_seq = 0, v_base_seq_prev = 0;
	/* File descriptors */
	socklen_t addrlen;
	struct sockaddr_in remote;
	int resfd = 0, bytes = 0;
	struct pollfd fds[2];
	char buffer[1500];
	memset(buffer, 0, 1500);
	/* Loop */
	int num = 0;
	janus_ultrasound_rtp_relay_packet packet;
    while(!g_atomic_int_get(&stopping)) {
		/* Prepare poll */
		num = 0;
		if(video_fd != -1) {
			fds[num].fd = video_fd;
			fds[num].events = POLLIN;
			fds[num].revents = 0;
			num++;
		}
		/* Wait for some data */
		resfd = poll(fds, num, 1000);
		if(resfd < 0) {
			JANUS_LOG(LOG_ERR, "[%s] Error polling... %d (%s)\n", mountpoint->name, errno, strerror(errno));
			mountpoint->enabled = FALSE;
			break;
		} else if(resfd == 0) {
			/* No data, keep going */
			continue;
		}
		int i = 0;
		for(i=0; i<num; i++) {
			if(fds[i].revents & (POLLERR | POLLHUP)) {
				/* Socket error? */
				JANUS_LOG(LOG_ERR, "[%s] Error polling: %s... %d (%s)\n", mountpoint->name,
					fds[i].revents & POLLERR ? "POLLERR" : "POLLHUP", errno, strerror(errno));
				mountpoint->enabled = FALSE;
				break;
			} else if(fds[i].revents & POLLIN) {
				/* Got an RTP packet */
                if(video_fd != -1 && fds[i].fd == video_fd) {
					/* Got something video (RTP) */
					if(mountpoint->active == FALSE)
						mountpoint->active = TRUE;
                    mountpoint->last_received_video = janus_get_monotonic_time();
					addrlen = sizeof(remote);
					bytes = recvfrom(video_fd, buffer, 1500, 0, (struct sockaddr*)&remote, &addrlen);
					//~ JANUS_LOG(LOG_VERB, "************************\nGot %d bytes on the video channel...\n", bytes);
					rtp_header *rtp = (rtp_header *)buffer;

					/* If paused, ignore this packet */
					if(!mountpoint->enabled)
						continue;
					//~ JANUS_LOG(LOG_VERB, " ... parsed RTP packet (ssrc=%u, pt=%u, seq=%u, ts=%u)...\n",
						//~ ntohl(rtp->ssrc), rtp->type, ntohs(rtp->seq_number), ntohl(rtp->timestamp));
					/* Relay on all sessions */
					packet.data = rtp;
                    packet.length = bytes;
					/* Do we have a new stream? */
					if(ntohl(packet.data->ssrc) != v_last_ssrc) {
						v_last_ssrc = ntohl(packet.data->ssrc);
						JANUS_LOG(LOG_INFO, "[%s] New video stream! (ssrc=%u)\n", name, v_last_ssrc);
						v_base_ts_prev = v_last_ts;
						v_base_ts = ntohl(packet.data->timestamp);
						v_base_seq_prev = v_last_seq;
						v_base_seq = ntohs(packet.data->seq_number);
					}
					v_last_ts = (ntohl(packet.data->timestamp)-v_base_ts)+v_base_ts_prev+4500;	/* FIXME We're assuming 15fps here... */
					packet.data->timestamp = htonl(v_last_ts);
					v_last_seq = (ntohs(packet.data->seq_number)-v_base_seq)+v_base_seq_prev+1;
					packet.data->seq_number = htons(v_last_seq);
					//~ JANUS_LOG(LOG_VERB, " ... updated RTP packet (ssrc=%u, pt=%u, seq=%u, ts=%u)...\n",
						//~ ntohl(rtp->ssrc), rtp->type, ntohs(rtp->seq_number), ntohl(rtp->timestamp));
					packet.data->type = mountpoint->codecs.video_pt;
					/* Backup the actual timestamp and sequence number set by the restreamer, in case switching is involved */
					packet.timestamp = ntohl(packet.data->timestamp);
					packet.seq_number = ntohs(packet.data->seq_number);
					/* Go! */
					janus_mutex_lock(&mountpoint->mutex);
					g_list_foreach(mountpoint->listeners, janus_ultrasound_relay_rtp_packet, &packet);
					janus_mutex_unlock(&mountpoint->mutex);
					continue;
				}
			}
		}
	}

	/* Notify users this mountpoint is done */
	janus_mutex_lock(&mountpoint->mutex);
	GList *viewer = g_list_first(mountpoint->listeners);
	/* Prepare JSON event */
	json_t *event = json_object();
	json_object_set_new(event, "ultrasound", json_string("event"));
	json_t *result = json_object();
	json_object_set_new(result, "status", json_string("stopped"));
	json_object_set_new(event, "result", result);
	char *event_text = json_dumps(event, JSON_INDENT(3) | JSON_PRESERVE_ORDER);
	json_decref(event);
	while(viewer) {
		janus_ultrasound_session *session = (janus_ultrasound_session *)viewer->data;
		if(session != NULL) {
			session->stopping = TRUE;
            session->started = FALSE;
			session->mountpoint = NULL;
			/* Tell the core to tear down the PeerConnection, hangup_media will do the rest */
			gateway->push_event(session->handle, &janus_ultrasound_plugin, NULL, event_text, NULL, NULL);
			gateway->close_pc(session->handle);
		}
		mountpoint->listeners = g_list_remove_all(mountpoint->listeners, session);
		viewer = g_list_first(mountpoint->listeners);
	}
	g_free(event_text);
	janus_mutex_unlock(&mountpoint->mutex);

    delete mountpoint;

	JANUS_LOG(LOG_VERB, "[%s] Leaving ultrasound relay thread\n", name);
	g_free(name);
	g_thread_unref(g_thread_self());
	return NULL;
}

static void janus_ultrasound_relay_rtp_packet(gpointer data, gpointer user_data) {
	janus_ultrasound_rtp_relay_packet *packet = (janus_ultrasound_rtp_relay_packet *)user_data;
	if(!packet || !packet->data || packet->length < 1) {
		JANUS_LOG(LOG_ERR, "Invalid packet...\n");
		return;
	}
	janus_ultrasound_session *session = (janus_ultrasound_session *)data;
	if(!session || !session->handle) {
		//~ JANUS_LOG(LOG_ERR, "Invalid session...\n");
		return;
	}

	/* Make sure there hasn't been a publisher switch by checking the SSRC */

    if(ntohl(packet->data->ssrc) != session->context.v_last_ssrc) {
        session->context.v_last_ssrc = ntohl(packet->data->ssrc);
        session->context.v_base_ts_prev = session->context.v_last_ts;
        session->context.v_base_ts = packet->timestamp;
        session->context.v_base_seq_prev = session->context.v_last_seq;
        session->context.v_base_seq = packet->seq_number;
    }
    /* Compute a coherent timestamp and sequence number */
    session->context.v_last_ts = (packet->timestamp-session->context.v_base_ts)
        + session->context.v_base_ts_prev+4500;	/* FIXME When switching, we assume 15fps */
    session->context.v_last_seq = (packet->seq_number-session->context.v_base_seq)+session->context.v_base_seq_prev+1;
    /* Update the timestamp and sequence number in the RTP packet, and send it */
    packet->data->timestamp = htonl(session->context.v_last_ts);
    packet->data->seq_number = htons(session->context.v_last_seq);
    if(gateway != NULL)
        gateway->relay_rtp(session->handle, true, (char *)packet->data, packet->length);
    /* Restore the timestamp and sequence number to what the publisher set them to */
    packet->data->timestamp = htonl(packet->timestamp);
    packet->data->seq_number = htons(packet->seq_number);


	return;
}

void Plugin::handleMessageReady(janus_ultrasound_session *session, janus_ultrasound_message* msg, json_t* root) {
    us_plugin->onSessionReady(msg->handle);
}

void Plugin::handleMessageWatch(janus_ultrasound_session *session, janus_ultrasound_message* msg, json_t* root) {

    /* RTP live source (e.g., from gstreamer/ffmpeg/vlc/etc.) */
    char* desc = "ULTRASOUND";
    char* secret = "";
    int pin = 0;
    int vport = us_plugin->getSessionPort(msg->handle);
    int vcodec = 100;
    char* vrtpmap = "VP8/90000";
    gboolean is_private = false;


    RTPSource* mp = janus_ultrasound_create_rtp_source(
        1,
        desc,
        vport,
        vcodec,
        vrtpmap
    );

    mp->secret = g_strdup(secret);

    session->stopping = FALSE;
    session->mountpoint = mp;

    /* TODO Check if user is already watching a stream, if the video is active, etc. */
    janus_mutex_lock(&mp->mutex);
    mp->listeners = g_list_append(mp->listeners, session);
    janus_mutex_unlock(&mp->mutex);
    char* sdp_type = "offer";	/* We're always going to do the offer ourselves, never answer */

    char* sdp = mp->createSDP();

    json_t* result = json_object();
    json_object_set_new(result, "status", json_string("preparing"));

    Plugin::sendPostMessageEvent(result, msg, sdp, sdp_type);
}

janus_plugin_result* Plugin::onMessage(janus_plugin_session *handle, char *transaction,
                                     char *message, char *sdp_type, char *sdp)
{

    if(g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized)) {
        return janus_plugin_result_new(JANUS_PLUGIN_ERROR,
               g_atomic_int_get(&stopping) ? "Shutting down" : "Plugin not initialized");
    }

    if(message == NULL || handle ->plugin_handle == NULL ||
      ((janus_ultrasound_session *)handle->plugin_handle)->destroyed)
    {
        return Plugin::handleMessageError(handle, transaction, message, NULL, sdp_type, sdp, "Empty message");
    }

    json_t *root = json_loads(message, 0, NULL);
    if(!root || !json_is_object(root)) {
        return Plugin::handleMessageError(handle, transaction, message, root, sdp_type, sdp, "JSON Error");
    }

    json_t *request = json_object_get(root, "request");
    if(!request || !json_is_string(request)) {
        return Plugin::handleMessageError(handle, transaction, message, root, sdp_type, sdp, "Missing element (request)");
    }

    Plugin::addMessageToQueue(handle, transaction, root, sdp_type, sdp);

    return janus_plugin_result_new(JANUS_PLUGIN_OK_WAIT, NULL);
}

janus_plugin_result* Plugin::handleMessageError(janus_plugin_session *handle, char *transaction, char* message,
                                        json_t *root, char *sdp_type, char *sdp, char* error)
{
    if(root != NULL) json_decref(root);
    free(transaction);
    free(message);
    free(sdp_type);
    free(sdp);

    /* Prepare JSON error event */
    json_t *event = json_object();
    json_object_set_new(event, "ultrasound", json_string("event"));
    json_object_set_new(event, "error", json_string(error));
    char *event_text = json_dumps(event, JSON_INDENT(3) | JSON_PRESERVE_ORDER);
    json_decref(event);
    janus_plugin_result *result = janus_plugin_result_new(JANUS_PLUGIN_OK, event_text);
    free(event_text);
    return result;
}

void Plugin::addMessageToQueue(janus_plugin_session *handle, char *transaction, json_t *root, char *sdp_type, char *sdp) {
    janus_ultrasound_message *msg = g_malloc0(sizeof(janus_ultrasound_message));
    msg->handle = handle;
    msg->transaction = transaction;
    msg->message = root;
    msg->sdp_type = sdp_type;
    msg->sdp = sdp;

    g_async_queue_push(messages, msg);
}

janus_plugin_result* janus_ultrasound_handle_message(janus_plugin_session *handle, char *transaction, char *message, char *sdp_type, char *sdp) {
    return Plugin::onMessage(handle, transaction, message, sdp_type, sdp);
}

void janus_ultrasound_incoming_data(janus_plugin_session *handle, char* buffer, int length) {
    Plugin::incomingData(handle, buffer, length);
}

void Plugin::incomingData(janus_plugin_session *handle, char* buffer, int length) {
    // Check plugin health
    if (handle == NULL || handle->stopped || g_atomic_int_get(&stopping) ||
        !g_atomic_int_get(&initialized) || gateway == NULL)
    {
        return;
    }

    // Check session health
    janus_ultrasound_session *session = (janus_ultrasound_session *)handle->plugin_handle;
    if (session == NULL || session->destroyed) {
        JANUS_LOG(LOG_ERR, "No session associated with this handle...\n");
        return;
    }

    // Check message
    if (buffer == NULL || length <= 0) {
        return;
    }

    // Pass on null-terminated string
    char* msg = (char*) malloc(length+1);
    memcpy(msg, buffer, length);
    msg[length] = '\0';

    us_plugin->onDataReceived(handle, msg);

    free(msg);
}

