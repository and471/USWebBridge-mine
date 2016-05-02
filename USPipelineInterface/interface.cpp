#include <cstdio>
#include <functional>
#include <USPipeline/GstUltrasoundImagePipeline.h>
#include <USPipeline/DNLImageSource.h>
#include <USPipeline/DNLFileImageSource.h>
#include "UltrasoundPlugin.h"
#include <gstreamermm.h>

#include "PatientMetadata.h"
#include "interface.h"


USPipelineInterface::USPipelineInterface() {
    int z = 0;
    int &argc = z;

    char** h = NULL;
    char** &argv = h;

    Gst::init(argc, argv);

    std::string folder = "/home/andrew/Project/forAndrew3D";

    DNLImageSource* dnl_image_source = new DNLFileImageSource(folder);
    pipeline = new GstUltrasoundImagePipeline(this);
    ((GstUltrasoundImagePipeline*)pipeline)->setDNLImageSource(dnl_image_source);
}

void USPipelineInterface::start() {
    pipeline->start();
}

USPipelineInterface::~USPipelineInterface() {
    delete pipeline;
}

void USPipelineInterface::setPlugin(UltrasoundPlugin* plugin) {
    this->plugin = plugin;

    this->plugin->setOnSetSliceCallback(
        std::bind(&USPipelineInterface::onSetSlice, this, std::placeholders::_1)
    );
}

void USPipelineInterface::setOnNewPatientMetadataCallback(std::function<void(PatientMetadata)> cb) {
    this->onNewPatientMetadataCallback = cb;
}

void USPipelineInterface::onNewPatientMetadata(PatientMetadata patient) {
    this->onNewPatientMetadataCallback(patient);
}

void USPipelineInterface::setOnNSlicesChangedCallback(std::function<void(int)> cb) {
    this->onNSlicesChangedCallback = cb;
}

void USPipelineInterface::onNSlicesChanged(int nSlices) {
    this->onNSlicesChangedCallback(nSlices);
}

void USPipelineInterface::onSetSlice(int slice) {
    onSetSliceCallback(slice);
}

void USPipelineInterface::setOnSetSliceCallback(std::function<void(int)> cb) {
    this->onSetSliceCallback = cb;
}
