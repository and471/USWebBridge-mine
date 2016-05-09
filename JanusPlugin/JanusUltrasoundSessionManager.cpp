#include <USPipelineInterface/FrameSource.h>
#include <GstPipeline/GstUltrasoundImagePipeline.h>
#include <DNLFrameSource/DNLFileFrameSource.h>
#include "JanusUltrasoundSession.h"
#include "JanusUltrasoundSessionManager.h"

using namespace std::placeholders;


JanusUltrasoundSessionManager::~JanusUltrasoundSessionManager() {
    delete frame_source;

    {
        // Remove all sessions
        std::unique_lock<std::mutex> lock(sessions_mutex);
        for (auto entry : sessions) {
            delete entry.second;
        }
        sessions.clear();
    }
}

void JanusUltrasoundSessionManager::addSession(JanusUltrasoundSession* session, janus_plugin_session* handle) {
    UltrasoundImagePipeline* pipeline = createPipeline(session);
    session->setPipeline(pipeline);
    sessions[handle] = session;
}

void JanusUltrasoundSessionManager::destroySession(janus_plugin_session* handle) {
    sessions[handle]->stop();
    {
        std::unique_lock<std::mutex> lock(sessions_mutex);
        delete sessions[handle];
        sessions.erase(handle);
    }
}

void JanusUltrasoundSessionManager::onSessionReady(janus_plugin_session* handle) {
    sessions[handle]->start();
}

void JanusUltrasoundSessionManager::onDataReceived(janus_plugin_session* handle, char* msg) {
    sessions[handle]->onDataReceived(msg);
}

JanusUltrasoundSession* JanusUltrasoundSessionManager::getSession(janus_plugin_session* handle) {
    return sessions[handle];
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


