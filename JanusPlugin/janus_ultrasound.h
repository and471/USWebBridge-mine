#ifndef JANUS_ULTRASOUND_H
#define JANUS_ULTRASOUND_H

#include <jansson.h>

typedef struct janus_ultrasound_codecs {
    gint video_codec;
    gint video_pt;
    char *video_rtpmap;
    char *video_fmtp;
} janus_ultrasound_codecs;


class Plugin {

public:
    static void incomingData(janus_plugin_session *handle, char* buffer, int length);
};

#endif
