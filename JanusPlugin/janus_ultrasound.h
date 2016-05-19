#ifndef JANUS_ULTRASOUND_H
#define JANUS_ULTRASOUND_H

#include <json.hpp>
using json = nlohmann::json;

#include "RTPSource.h"
#include <thread>

class RTPSource;

extern "C" {
#include <janus/plugin.h>
}

/* Useful stuff */
static volatile gint initialized = 0, stopping = 0;
static janus_callbacks *gateway = NULL;

typedef struct janus_ultrasound_context {
    /* Needed to fix seq and ts in case of stream switching */
    uint32_t v_last_ssrc, v_last_ts, v_base_ts, v_base_ts_prev;
    uint16_t v_last_seq, v_base_seq, v_base_seq_prev;
} janus_ultrasound_context;


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

#endif
