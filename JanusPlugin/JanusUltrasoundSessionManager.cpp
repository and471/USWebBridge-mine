#include <USPipelineInterface/FrameSource.h>
#include <GstPipeline/GstUltrasoundImagePipeline.h>
#include <DNLFrameSource/DNLFileFrameSource.h>
#include "JanusUltrasoundSession.h"
#include "JanusUltrasoundSessionManager.h"

using namespace std::placeholders;

JanusUltrasoundSessionManager::JanusUltrasoundSessionManager(janus_callbacks* gateway)
{
    this->gateway = gateway;
    frame_source = createFrameSource();
}

JanusUltrasoundSessionManager::~JanusUltrasoundSessionManager() {
    delete frame_source;
}

void JanusUltrasoundSessionManager::newSession(janus_plugin_session* handle) {
    JanusUltrasoundSession* session = new JanusUltrasoundSession(gateway, handle);
    UltrasoundImagePipeline* pipeline = createPipeline(session);
    session->setPipeline(pipeline);

    sessions[handle] = session;
}

void JanusUltrasoundSessionManager::destroySession(janus_plugin_session* handle) {
    sessions[handle]->stop();
    delete sessions[handle];
    sessions.erase(handle);
    int p = 0;
}

void JanusUltrasoundSessionManager::onSessionReady(janus_plugin_session* handle) {
    sessions[handle]->start();
}

void JanusUltrasoundSessionManager::onDataReceived(janus_plugin_session* handle, char* msg) {
    sessions[handle]->onDataReceived(msg);
}

int JanusUltrasoundSessionManager::getSessionPort(janus_plugin_session* handle) {
    return sessions[handle]->getPort();
}

FrameSource* JanusUltrasoundSessionManager::createFrameSource() {
    std::string folder = "/home/andrew/Project/forAndrew3D";
    return new DNLFileFrameSource(folder);
}

UltrasoundImagePipeline* JanusUltrasoundSessionManager::createPipeline(UltrasoundController* controller) {
    GstUltrasoundImagePipeline* pipeline = new GstUltrasoundImagePipeline(controller);
    pipeline->setFrameSource(frame_source);
    return pipeline;
}


