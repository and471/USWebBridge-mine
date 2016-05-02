#include <USPipelineInterface/interface.h>
#include <USPipelineInterface/PatientMetadata.h>
#include <cstdio>
#include <functional>
#include <jansson.h>
#include <USPipelineInterface/UltrasoundPlugin.h>

#include "JanusUltrasoundPlugin.h"

using namespace std::placeholders;

JanusUltrasoundPlugin::JanusUltrasoundPlugin(USPipelineInterface* interface,
                                   janus_callbacks* gateway,
                                   janus_plugin_session* handle)
{
    this->interface = interface;

    interface->setOnNewPatientMetadataCallback(
        std::bind(&UltrasoundPlugin::onNewPatientMetadata, this, std::placeholders::_1)
    );
    interface->setOnNSlicesChangedCallback(
        std::bind(&UltrasoundPlugin::onNSlicesChanged, this, std::placeholders::_1)
    );

    this->gateway = gateway;
    this->handle = handle;
}


void JanusUltrasoundPlugin::start() {
    if (!started) {
        interface->start();
    }
}

void JanusUltrasoundPlugin::onNewPatientMetadata(PatientMetadata patient) {
    json_t* obj = json_object();
    json_object_set_new(obj, "name", json_string(patient.name.c_str()));

    this->sendMethod(obj, METHOD_NEW_PATIENT_METADATA);
    json_decref(obj);
}

void JanusUltrasoundPlugin::onNSlicesChanged(int nSlices) {
    json_t* obj = json_object();
    json_object_set_new(obj, "nSlices", json_integer(nSlices));

    this->sendMethod(obj, METHOD_N_SLICES_CHANGED);
    json_decref(obj);
}

void JanusUltrasoundPlugin::sendMethod(json_t* data, std::string method) {
    json_t* obj = json_object();
    json_object_set_new(obj, "method", json_string(method.c_str()));
    json_object_set(obj, "data", data);
    sendData(obj);
    json_decref(obj);
}

void JanusUltrasoundPlugin::sendData(json_t* obj) {
    char* str = json_dumps(obj, 0);
    gateway->relay_data(handle, str, strlen(str));
    free(str);
}

void JanusUltrasoundPlugin::onDataReceived(char* msg) {
    json_t* obj = json_loads(msg, 0, NULL);
    char* method = json_string_value(json_object_get(obj, "method"));

    json_t* data = json_object_get(obj, "data");


    if (strcmp(method, "SET_SLICE") == 0) {
        int slice = json_integer_value(json_object_get(data, "slice"));
        onSetSlice(slice);
    }

    json_decref(obj);
}

void JanusUltrasoundPlugin::onSetSlice(int slice) {
    onSetSliceCallback(slice);
}

void JanusUltrasoundPlugin::setOnSetSliceCallback(std::function<void(int)> cb) {
    this->onSetSliceCallback = cb;
}
