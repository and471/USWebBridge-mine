#ifndef JANUS_ULTRASOUNDPLUGIN_H
#define JANUS_ULTRASOUNDPLUGIN_H

#include <janus/plugin.h>
#include <USPipelineInterface/FrameSource.h>

#include "plugin_hooks.h"
#include "JanusUltrasoundSession.h"
#include <mutex>

class JanusUltrasoundSession; //forward declaration


class JanusUltrasoundSessionManager
{
public:
    JanusUltrasoundSessionManager(janus_callbacks* gateway);
    ~JanusUltrasoundSessionManager();

    void addSession(JanusUltrasoundSession* session, janus_plugin_session* handle);
    void destroySession(janus_plugin_session* handle);
    void onSessionReady(janus_plugin_session* handle);
    JanusUltrasoundSession* getSession(janus_plugin_session* handle);

    void onDataReceived(janus_plugin_session *handle, char* buffer, int length);
    janus_plugin_result* onMessage(janus_plugin_session *handle,
        char *transaction, char *message, char *sdp_type, char *sdp);
    void addMessageToQueue(JanusUltrasoundSession* session, char *transaction, json root,
                           char *sdp_type, char *sdp);
    void messageHandlerThread();
    janus_plugin_result* handleMessageError(janus_plugin_session *handle,
                                            char *transaction, char* message,
                                            json root, char *sdp_type, char *sdp, char* error);

    FrameSource* createFrameSource();
    UltrasoundImagePipeline* createPipeline(UltrasoundController* controller);


private:
    FrameSource* frame_source;
    janus_callbacks* gateway;

    std::queue<Message*> messages;
    std::mutex messages_mutex;
    std::thread* handler_thread;

    std::mutex sessions_mutex;
    std::map<janus_plugin_session*, JanusUltrasoundSession*> sessions;
};


#endif // JANUS_ULTRASOUNDPLUGIN_H
