#ifndef JANUS_ULTRASOUNDPLUGIN_H
#define JANUS_ULTRASOUNDPLUGIN_H

#include <janus/plugin.h>
#include <USPipelineInterface/FrameSource.h>

#include "plugin_hooks.h"
#include "JanusUltrasoundSession.h"
#include <mutex>


class JanusUltrasoundSessionManager
{
public:
    JanusUltrasoundSessionManager(janus_callbacks* gateway);
    ~JanusUltrasoundSessionManager();

    void addSession(JanusUltrasoundSession* session, janus_plugin_session* handle);
    void destroySession(janus_plugin_session* handle);
    void onSessionReady(janus_plugin_session* handle);
    JanusUltrasoundSession* getSession(janus_plugin_session* handle);

    void onDataReceived(janus_plugin_session* handle, char* msg);

    FrameSource* createFrameSource();
    UltrasoundImagePipeline* createPipeline(UltrasoundController* controller);


private:
    FrameSource* frame_source;
    janus_callbacks* gateway;

    std::mutex sessions_mutex;
    std::map<janus_plugin_session*, JanusUltrasoundSession*> sessions;
};

#endif // JANUS_ULTRASOUNDPLUGIN_H
