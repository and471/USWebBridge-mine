#include <cstdio>
#include <functional>
#include <USPipeline/GstUltrasoundImagePipeline.h>
#include "FrameSource.h"
#include <USPipeline/DNLFileFrameSource.h>
#include "UltrasoundPlugin.h"
#include <gstreamermm.h>

#include "PatientMetadata.h"
#include "interface.h"

static FrameSource* frameSource = nullptr;

FrameSource* getFrameSource() {
    if (frameSource == nullptr) {
        std::string folder = "/home/andrew/Project/forAndrew3D";
        frameSource = new DNLFileFrameSource(folder);
    }

    return frameSource;
}

UltrasoundImagePipeline* createPipeline(UltrasoundController* controller) {
    GstUltrasoundImagePipeline* pipeline = new GstUltrasoundImagePipeline(controller);
    return pipeline;
}
