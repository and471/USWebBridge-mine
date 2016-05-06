#include <cstdio>
#include <functional>
#include <jansson.h>

#include <USPipelineInterface/FrameSource.h>
#include <USPipelineInterface/UltrasoundImagePipeline.h>
#include <USPipelineInterface/UltrasoundController.h>

#include "DummyController.h"

using namespace std::placeholders;

DummyController::DummyController() {}

DummyController::~DummyController()
{
    delete this->pipeline;
}

void DummyController::setPipeline(UltrasoundImagePipeline *pipeline) {
    this->pipeline = pipeline;
    pipeline->setOnNewPatientMetadataCallback(
        std::bind(&UltrasoundController::onNewPatientMetadata, this, std::placeholders::_1)
    );
}

void DummyController::start() {
    if (!started) {
        pipeline->start();
        started = true;
    }
}

void DummyController::stop() {
    pipeline->stop();
}

void DummyController::onNewPatientMetadata(PatientMetadata patient) {
    
}

void DummyController::onNSlicesChanged(int nSlices) {
    
}

void DummyController::onSetSlice(int slice) {
    
}

void DummyController::setOnSetSliceCallback(std::function<void(int)> cb) {
    
}

