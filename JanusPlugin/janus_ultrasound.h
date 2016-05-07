#ifndef JANUS_ULTRASOUND_H
#define JANUS_ULTRASOUND_H

#include <jansson.h>
#include "RTPSource.h"

class RTPSource;

extern "C" {
#include <janus/plugin.h>
}

/* Useful stuff */
static volatile gint initialized = 0, stopping = 0;
static janus_callbacks *gateway = NULL;
static GThread *handler_thread;
typedef struct janus_ultrasound_message {
    janus_plugin_session *handle;
    char *transaction;
    json_t *message;
    char *sdp_type;
    char *sdp;
} janus_ultrasound_message;

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


class Plugin {

public:
    static void incomingData(janus_plugin_session *handle, char* buffer, int length);

    static janus_plugin_result* onMessage(janus_plugin_session *handle,
        char *transaction, char *message, char *sdp_type, char *sdp);

    static void addMessageToQueue(janus_plugin_session *handle, char *transaction,
                                  json_t *root, char *sdp_type, char *sdp);

    static janus_plugin_result* handleMessageError(janus_plugin_session *handle,
                                            char *transaction, char* message,
                                            json_t *root, char *sdp_type, char *sdp, char* error);

    static void handleMessageReady(janus_ultrasound_session *session, janus_ultrasound_message* msg, json_t* root);
    static void handleMessageWatch(janus_ultrasound_session *session, janus_ultrasound_message* msg, json_t* root);
    static void handleMessageStart(janus_ultrasound_session *session, janus_ultrasound_message* msg, json_t* root);
    static void handleMessageStop(janus_ultrasound_session* session, janus_ultrasound_message* msg, json_t* root);

    static void sendPostMessageEvent(json_t* result, janus_ultrasound_message* msg, char* sdp, char* sdp_type);
};

#endif
