#ifndef JANUS_ULTRASOUNDPLUGIN_H
#define JANUS_ULTRASOUNDPLUGIN_H

#include <janus/plugin.h>
#include <USPipelineInterface/interface.h>
#include <USPipelineInterface/FrameSource.h>

#include "plugin_hooks.h"
#include "JanusUltrasoundSession.h"


class JanusUltrasoundPlugin
{
public:
    JanusUltrasoundPlugin(janus_callbacks* gateway);
    ~JanusUltrasoundPlugin();

    void newSession(janus_plugin_session* handle);
    void destroySession(janus_plugin_session* handle);
    void onSessionReady(janus_plugin_session* handle);
    void onDataReceived(janus_plugin_session* handle, char* msg);

private:
    FrameSource* frame_source;
    janus_callbacks* gateway;
    std::map<janus_plugin_session*, JanusUltrasoundSession*> sessions;
};

#endif // JANUS_ULTRASOUNDPLUGIN_H
