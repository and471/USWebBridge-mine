#include <cstdio>
#include <gstreamermm.h>
#include <string>

#include "DummyController.h"

#include <USPipelineInterface/FrameSource.h>
#include <USPipelineInterface/UltrasoundImagePipeline.h>
#include <USPipelineInterface/UltrasoundController.h>
#include <DNLFrameSource/DNLFileFrameSource.h>
#include <GstPipeline/GstUltrasoundImagePipeline.h>

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Not enough arguments\n");
        printf("Usage:\n");
        printf("%s <folder>\n", argv[0]);
        return -1;
    }


    Gst::init(argc, argv);
    std::string folder = argv[1];

    FrameSource* frame_source = new DNLFileFrameSource(folder);
    UltrasoundController* controller = new DummyController();
    UltrasoundImagePipeline *pipeline = new GstUltrasoundImagePipeline(controller);
    pipeline->setFrameSource(frame_source);
    controller->setPipeline(pipeline);

    pipeline->start();

    std::string tmp;
    std::getline(std::cin, tmp);

    pipeline->stop();

    delete pipeline;
    delete controller;
    delete frame_source;

    return 0;
}
