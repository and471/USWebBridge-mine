#include <USPipelineInterface/FrameSource.h>
#include <GstPipeline/GstUltrasoundImagePipeline.h>
#include <DNLFrameSource/DNLFileFrameSource.h>
#include "JanusUltrasoundSession.h"
#include "JanusUltrasoundPlugin.h"

using namespace std::placeholders;

JanusUltrasoundPlugin::JanusUltrasoundPlugin(janus_callbacks* gateway)
{
    this->gateway = gateway;
    frame_source = createFrameSource();
}

JanusUltrasoundPlugin::~JanusUltrasoundPlugin() {
    delete frame_source;
}

void JanusUltrasoundPlugin::newSession(janus_plugin_session* handle) {
    JanusUltrasoundSession* session = new JanusUltrasoundSession(gateway, handle);
    UltrasoundImagePipeline* pipeline = createPipeline(session);
    session->setPipeline(pipeline);

    sessions[handle] = session;
}

void JanusUltrasoundPlugin::destroySession(janus_plugin_session* handle) {
    sessions[handle]->stop();
    delete sessions[handle];
    sessions.erase(handle);
    int p = 0;
}

void JanusUltrasoundPlugin::onSessionReady(janus_plugin_session* handle) {
    sessions[handle]->start();
}

void JanusUltrasoundPlugin::onDataReceived(janus_plugin_session* handle, char* msg) {
    sessions[handle]->onDataReceived(msg);
}

int JanusUltrasoundPlugin::getSessionPort(janus_plugin_session* handle) {
    return sessions[handle]->getPort();
}

FrameSource* JanusUltrasoundPlugin::createFrameSource() {
    std::string folder = "/home/andrew/Project/forAndrew3D";
    return new DNLFileFrameSource(folder);
}

UltrasoundImagePipeline* JanusUltrasoundPlugin::createPipeline(UltrasoundController* controller) {
    GstUltrasoundImagePipeline* pipeline = new GstUltrasoundImagePipeline(controller);
    pipeline->setFrameSource(frame_source);
    return pipeline;
}


