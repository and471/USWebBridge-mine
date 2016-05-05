#include <USPipelineInterface/interface.h>
#include <USPipelineInterface/FrameSource.h>
#include "JanusUltrasoundSession.h"
#include "JanusUltrasoundPlugin.h"

using namespace std::placeholders;

JanusUltrasoundPlugin::JanusUltrasoundPlugin(janus_callbacks* gateway)
{
    this->gateway = gateway;
    this->frame_source = getFrameSource();
}

void JanusUltrasoundPlugin::newSession(janus_plugin_session* handle) {
    JanusUltrasoundSession* session = new JanusUltrasoundSession(gateway, handle);
    sessions[handle] = session;
}

void JanusUltrasoundPlugin::destroySession(janus_plugin_session* handle) {
    sessions[handle]->stop();
    delete sessions[handle];
    sessions.erase(handle);
}

void JanusUltrasoundPlugin::onSessionReady(janus_plugin_session* handle) {
    sessions[handle]->start();
}

void JanusUltrasoundPlugin::onDataReceived(janus_plugin_session* handle, char* msg) {
    sessions[handle]->onDataReceived(msg);
}


