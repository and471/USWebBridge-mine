#ifndef JANUS_ULTRASOUNDPLUGIN_H
#define JANUS_ULTRASOUNDPLUGIN_H

#include <janus/plugin.h>
#include "plugin_hooks.h"
#include <USPipelineInterface/UltrasoundPlugin.h>

#include <USPipelineInterface/interface.h>
#include <USPipelineInterface/PatientMetadata.h>
#include <functional>
#include <jansson.h>

static const std::string METHOD_NEW_PATIENT_METADATA = "NEW_PATIENT_METADATA";
static const std::string METHOD_N_SLICES_CHANGED = "N_SLICES_CHANGED";

class JanusUltrasoundPlugin : public UltrasoundPlugin
{
public:
    JanusUltrasoundPlugin(janus_callbacks* gateway, janus_plugin_session* handle);
    ~JanusUltrasoundPlugin();

    void setPipeline(UltrasoundImagePipeline* pipeline);

    void sendMethod(json_t* data, std::string method);
    void sendData(json_t* obj);
    void onDataReceived(char* data);

    // Virtual methods
    void start();
    void onNewPatientMetadata(PatientMetadata patient);
    void onNSlicesChanged(int nSlices);
    void onSetSlice(int slice);
    void setOnSetSliceCallback(std::function<void(int)> cb);

private:
    USPipelineInterface* interface;
    janus_callbacks* gateway;
    janus_plugin_session* handle;
    bool started = false;

    std::function<void(int)> onSetSliceCallback;

};

#endif // JANUS_ULTRASOUNDPLUGIN_H
