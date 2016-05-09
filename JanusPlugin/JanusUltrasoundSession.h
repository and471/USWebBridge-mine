#ifndef JANUS_ULTRASOUNDSESSION_H
#define JANUS_ULTRASOUNDSESSION_H

#include <janus/plugin.h>
#include <functional>

#include <json.hpp>
using json = nlohmann::json;

#include <USPipelineInterface/PatientMetadata.h>
#include <USPipelineInterface/FrameSource.h>
#include <USPipelineInterface/UltrasoundImagePipeline.h>
#include <USPipelineInterface/UltrasoundController.h>
#include "RTPSource.h"

#include "plugin_hooks.h"

static const std::string METHOD_NEW_PATIENT_METADATA = "NEW_PATIENT_METADATA";
static const std::string METHOD_N_SLICES_CHANGED = "N_SLICES_CHANGED";

class JanusUltrasoundSession : public UltrasoundController
{
public:
    JanusUltrasoundSession(janus_callbacks* gateway, janus_plugin_session* handle);
    ~JanusUltrasoundSession();

    void onDataReceived(char* data);
    int getPort();
    void sendMethod(json data, std::string method);
    void sendData(json obj);

    void handleMessageReady(Message* msg);
    void handleMessageWatch(Message* msg);
    void handleMessageStart(Message* msg);
    void handleMessageStop(Message* msg);

    void tearDownPeerConnection();

    // Virtual methods
    void start();
    void stop();
    void onNewPatientMetadata(PatientMetadata patient);
    void onNSlicesChanged(int nSlices);
    void onSetSlice(int slice);
    void setOnSetSliceCallback(std::function<void(int)> cb);
    void setPipeline(UltrasoundImagePipeline* pipeline);


    RTPSource* mountpoint;

    janus_ultrasound_context context;
    gboolean stopping;
    volatile gint hangingup;
    gint64 destroyed;
    bool started = false;

    janus_callbacks* gateway;
    janus_plugin_session* handle;

private:
    bool pipeline_started = false;
    std::function<void(int)> onSetSliceCallback;

};

#endif // JANUS_ULTRASOUNDSESSION_H
