#include <USPipelineInterface/FrameSource.h>
#include <GstPipeline/GstUltrasoundImagePipeline.h>
#include <DNLFrameSource/DNLFileFrameSource.h>
#include "JanusUltrasoundSession.h"
#include "JanusUltrasoundSessionManager.h"
#include "ratecontrol/TFRCController.h"

using namespace std::placeholders;

void JanusUltrasoundSessionManager::addSession(JanusUltrasoundSession* session, janus_plugin_session* handle) {
    UltrasoundImagePipeline* pipeline = createPipeline(session);
    frame_source->start();
    session->setPipeline(pipeline);

    RateController* rateController = new TFRCController();
    session->setRateController(rateController);

    sessions[handle] = session;
}

void JanusUltrasoundSessionManager::destroySession(janus_plugin_session* handle) {
    sessions[handle]->stop();
    {
        std::unique_lock<std::mutex> lock(sessions_mutex);
        delete sessions[handle];
        sessions.erase(handle);
    }

    if (sessions.empty()) {
        frame_source->stop();
    }

}

void JanusUltrasoundSessionManager::onRTCP(janus_plugin_session* handle, char* packet, int len, struct timeval arrival) {
    getSession(handle)->onRTCP(packet, len, arrival);
}

JanusUltrasoundSession* JanusUltrasoundSessionManager::getSession(janus_plugin_session* handle) {
    return sessions[handle];
}

FrameSource* JanusUltrasoundSessionManager::createFrameSource() {
    std::string folder = std::string(std::getenv("DNLIMAGESOURCE"));
    return new DNLFileFrameSource(folder);
}

UltrasoundImagePipeline* JanusUltrasoundSessionManager::createPipeline(UltrasoundController* controller) {
    GstUltrasoundImagePipeline* pipeline = new GstUltrasoundImagePipeline(controller);
    pipeline->setFrameSource(frame_source);
    return pipeline;
}


