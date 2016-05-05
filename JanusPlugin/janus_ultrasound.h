#ifndef JANUS_ULTRASOUND_H
#define JANUS_ULTRASOUND_H

#include <jansson.h>

class Plugin {

public:
    static void incomingData(janus_plugin_session *handle, char* buffer, int length);
};

#endif
