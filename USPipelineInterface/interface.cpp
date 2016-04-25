#include <cstdio>
#include <USPipeline/UltrasoundImagePipeline.h>
#include <USPipeline/DNLImageSource.h>
#include <USPipeline/DNLFileImageSource.h>

#include "interface.h"

USPipelineInterface::USPipelineInterface() {
    int argc = 0;
    gst_init(&argc, NULL);

    std::string folder = "/home/andrew/Project/forAndrew2D";

    this->callbacks = new std::map<int, void (*)(void*)>();

    DNLImageSource* dnl_image_source = new DNLFileImageSource(folder);
    pipeline = new UltrasoundImagePipeline(this);
    pipeline->setDNLImageSource(dnl_image_source);
    pipeline->start();
}

USPipelineInterface::~USPipelineInterface() {
    delete pipeline;
}

void USPipelineInterface::connect(int signal, void (*func)(void*)) {
    (*(this->callbacks))[signal] = func;
}

void USPipelineInterface::fire(int signal, void* data) {
    this->callbacks->at(signal)(data);
}
