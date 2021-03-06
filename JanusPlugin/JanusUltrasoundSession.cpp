#include <cstdio>
#include <functional>
#include <jansson.h>

#include <json.hpp>
using json = nlohmann::json;

#include <USPipelineInterface/FrameSource.h>
#include <USPipelineInterface/UltrasoundImagePipeline.h>
#include <USPipelineInterface/UltrasoundController.h>

#include "JanusUltrasoundSession.h"

using namespace std::placeholders;


void JanusUltrasoundSession::setPipeline(UltrasoundImagePipeline *pipeline) {
    this->pipeline = pipeline;
    pipeline->setOnNewPatientMetadataCallback(
        std::bind(&JanusUltrasoundSession::onNewPatientMetadata, this, std::placeholders::_1)
    );
    pipeline->setOnNewImageMetadataCallback(
        std::bind(&JanusUltrasoundSession::onNewImageMetadata, this, std::placeholders::_1)
    );
}

void JanusUltrasoundSession::start() {
    if (!pipeline_started) {
        pipeline->start();
        pipeline_started = true;
    }
}

void JanusUltrasoundSession::stop() {
    pipeline->stop();
}

void JanusUltrasoundSession::onNewPatientMetadata(PatientMetadata patient) {
    json obj;
    obj["name"] = patient.name;

    sendMethod(obj, METHOD_NEW_PATIENT_METADATA);
}

void JanusUltrasoundSession::onNewImageMetadata(ImageMetadata metadata) {
    json obj;

    std::vector<double> position(std::begin(metadata.position), std::end(metadata.position));
    std::vector<double> orientation(std::begin(metadata.orientation), std::end(metadata.orientation));
    std::vector<double> forces(std::begin(metadata.forces), std::end(metadata.forces));

    obj["position"] = position;
    obj["orientation"] = orientation;
    obj["spacing"] = metadata.spacing;
    obj["forces"] = forces;

    sendMethod(obj, METHOD_NEW_IMAGE_METADATA);
}

void JanusUltrasoundSession::onNSlicesChanged(int nSlices) {
    json obj;
    obj["nSlices"] = nSlices;

    this->sendMethod(obj, METHOD_N_SLICES_CHANGED);
}

void JanusUltrasoundSession::sendMethod(json data, std::string method) {
    json obj;
    obj["method"] = method;
    obj["data"] = data;
    sendData(obj);
}

void JanusUltrasoundSession::sendData(json obj) {
    std::string str = obj.dump();
    char* s = str.c_str();
    gateway->relay_data(handle, s, strlen(s));
}


void JanusUltrasoundSession::onDataReceived(char* msg) {
    json obj = json::parse(msg);

    if (obj["method"] == "SET_SLICE") {
        int slice = obj["data"]["slice"];
        onSetSlice(slice);
    } else if (obj["method"] == "CROP") {
        int left = obj["data"]["left"];
        int right = obj["data"]["right"];
        int top = obj["data"]["top"];
        int bottom = obj["data"]["bottom"];
        pipeline->crop(left, right, top, bottom);
    }
}

int JanusUltrasoundSession::getPort() {
    return pipeline->getPort();
}

void JanusUltrasoundSession::onSetSlice(int slice) {
    onSetSliceCallback(slice);
}

void JanusUltrasoundSession::setOnSetSliceCallback(std::function<void(int)> cb) {
    this->onSetSliceCallback = cb;
}

void JanusUltrasoundSession::tearDownPeerConnection() {
    json event;
    event["ultrasound"] = "event";
    event["result"]["status"] = "stopped";

    std::string event_str = event.dump();
    gateway->push_event(handle, &janus_ultrasound_plugin, NULL, event_str.c_str(), NULL, NULL);
    gateway->close_pc(handle);
}

void JanusUltrasoundSession::onRTCP(char* packet, int len, struct timeval arrival) {
    int total = len;

    // Parse Receiver Response RTCP packets
    rtcp_header *rtcp = (rtcp_header*) packet;
    while(rtcp) {
        if (rtcp->type == RTCP_RR) {
            rateController->onRTCPReceiverReport((rtcp_rr*) rtcp, arrival);
        }

        /* If this is a compound packet, move onto next one */
        int length = ntohs(rtcp->length);
        total -= length*4+4;

        if (length == 0 || total <= 0) {
            // No more left to process
            break;
        }
        rtcp = (rtcp_header *)((uint32_t*)rtcp + length + 1);
    }
}

void JanusUltrasoundSession::onRateControllerBitrateChange(int bitrate) {
    pipeline->setBitrate(bitrate);
}

void JanusUltrasoundSession::setRateController(RateController* rateController) {
    this->rateController = rateController;
    rateController->setOnBitrateChangeCallback(
        std::bind(&JanusUltrasoundSession::onRateControllerBitrateChange, this, std::placeholders::_1)
    );
}
