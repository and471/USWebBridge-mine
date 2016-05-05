#include <cstdio>
#include <functional>
#include <jansson.h>

#include <USPipelineInterface/FrameSource.h>
#include <USPipelineInterface/UltrasoundImagePipeline.h>
#include <USPipelineInterface/UltrasoundController.h>

#include "JanusUltrasoundSession.h"

using namespace std::placeholders;

JanusUltrasoundSession::JanusUltrasoundSession(janus_callbacks* gateway, janus_plugin_session* handle)
{
    this->gateway = gateway;
    this->handle = handle;
}

JanusUltrasoundSession::~JanusUltrasoundSession()
{
    delete this->pipeline;
}

void JanusUltrasoundSession::setPipeline(UltrasoundImagePipeline *pipeline) {
    this->pipeline = pipeline;
    pipeline->setOnNewPatientMetadataCallback(
        std::bind(&UltrasoundController::onNewPatientMetadata, this, std::placeholders::_1)
    );
}

void JanusUltrasoundSession::start() {
    if (!started) {
        pipeline->start();
    }
}

void JanusUltrasoundSession::stop() {
    pipeline->stop();
}

void JanusUltrasoundSession::onNewPatientMetadata(PatientMetadata patient) {
    json_t* obj = json_object();
    json_object_set_new(obj, "name", json_string(patient.name.c_str()));

    this->sendMethod(obj, METHOD_NEW_PATIENT_METADATA);
    json_decref(obj);
}

void JanusUltrasoundSession::onNSlicesChanged(int nSlices) {
    json_t* obj = json_object();
    json_object_set_new(obj, "nSlices", json_integer(nSlices));

    this->sendMethod(obj, METHOD_N_SLICES_CHANGED);
    json_decref(obj);
}

void JanusUltrasoundSession::sendMethod(json_t* data, std::string method) {
    json_t* obj = json_object();
    json_object_set_new(obj, "method", json_string(method.c_str()));
    json_object_set(obj, "data", data);
    sendData(obj);
    json_decref(obj);
}

void JanusUltrasoundSession::sendData(json_t* obj) {
    char* str = json_dumps(obj, 0);
    gateway->relay_data(handle, str, strlen(str));
    free(str);
}

void JanusUltrasoundSession::onDataReceived(char* msg) {
    json_t* obj = json_loads(msg, 0, NULL);
    if (obj == NULL) return;

    char* method = json_string_value(json_object_get(obj, "method"));

    json_t* data = json_object_get(obj, "data");

    if (strcmp(method, "SET_SLICE") == 0) {
        int slice = json_integer_value(json_object_get(data, "slice"));
        onSetSlice(slice);
    }

    json_decref(obj);
}

void JanusUltrasoundSession::onSetSlice(int slice) {
    onSetSliceCallback(slice);
}

void JanusUltrasoundSession::setOnSetSliceCallback(std::function<void(int)> cb) {
    this->onSetSliceCallback = cb;
}

