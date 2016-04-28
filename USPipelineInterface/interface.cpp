#include <cstdio>
#include <functional>
#include <USPipeline/UltrasoundImagePipeline.h>
#include <USPipeline/DNLImageSource.h>
#include <USPipeline/DNLFileImageSource.h>

#include "PatientMetadata.h"
#include "interface.h"


USPipelineInterface::USPipelineInterface() {
    int argc = 0;
    gst_init(&argc, NULL);

    std::string folder = "/home/andrew/Project/forAndrew2D";

    DNLImageSource* dnl_image_source = new DNLFileImageSource(folder);
    pipeline = new UltrasoundImagePipeline(this);
    pipeline->setDNLImageSource(dnl_image_source);
}

void USPipelineInterface::start() {
    pipeline->start();
}

USPipelineInterface::~USPipelineInterface() {
    delete pipeline;
}

void USPipelineInterface::setOnNewPatientMetadataCallback(std::function<void(PatientMetadata)> cb) {
    this->onNewPatientMetadataCallback = cb;
}

void USPipelineInterface::OnNewPatientMetadata(PatientMetadata patient) {
    this->onNewPatientMetadataCallback(patient);
}
