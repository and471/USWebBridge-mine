#ifndef JANUS_ULTRASOUND_H
#define JANUS_ULTRASOUND_H

#include <json.hpp>
using json = nlohmann::json;

#include "RTPSource.h"

class RTPSource;

extern "C" {
#include <janus/plugin.h>
}

/* Useful stuff */
static volatile gint initialized = 0, stopping = 0;
static janus_callbacks *gateway = NULL;
static GThread *handler_thread;


typedef struct janus_ultrasound_context {
    /* Needed to fix seq and ts in case of stream switching */
    uint32_t v_last_ssrc, v_last_ts, v_base_ts, v_base_ts_prev;
    uint16_t v_last_seq, v_base_seq, v_base_seq_prev;
} janus_ultrasound_context;

typedef struct janus_ultrasound_session {
    janus_callbacks* gateway;
    janus_plugin_session *handle;
    RTPSource* mountpoint;
    gboolean started;
    janus_ultrasound_context context;
    gboolean stopping;
    volatile gint hangingup;
    gint64 destroyed;	/* Time at which this session was marked as destroyed */
} janus_ultrasound_session;

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


class Message {

public:
    Message(janus_plugin_session* handle, char *transaction, json message, char *sdp, char *sdp_type);
    ~Message();

    janus_plugin_session *handle;
    char *transaction;
    json message;
    char *sdp;
    char *sdp_type;
};

class Plugin {

public:
    static void incomingData(janus_plugin_session *handle, char* buffer, int length);

    static janus_plugin_result* onMessage(janus_plugin_session *handle,
        char *transaction, char *message, char *sdp_type, char *sdp);

    static void addMessageToQueue(janus_plugin_session *handle, char *transaction,
                                  json root, char *sdp_type, char *sdp);

    static janus_plugin_result* handleMessageError(janus_plugin_session *handle,
                                            char *transaction, char* message,
                                            json root, char *sdp_type, char *sdp, char* error);

    static void handleMessageReady(janus_ultrasound_session *session, Message* msg);
    static void handleMessageWatch(janus_ultrasound_session *session, Message* msg);
    static void handleMessageStart(janus_ultrasound_session *session, Message* msg);
    static void handleMessageStop(janus_ultrasound_session* session, Message* msg);

    static void sendPostMessageEvent(json result, Message* msg, char* sdp, char* sdp_type);
};

/* Helper to create an RTP live source (e.g., from gstreamer/ffmpeg/vlc/etc.) */
RTPSource* janus_ultrasound_create_rtp_source(uint64_t id, char *name, uint16_t vport,
                                              uint8_t vcodec, char *vrtpmap);
/* Useful stuff */
static void *janus_ultrasound_handler(void *data);
void janus_ultrasound_message_free(Message *msg);

#endif
