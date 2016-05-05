#include <cstdio>
#include <gstreamermm.h>
#include <string>

#include <USPipelineInterface/FrameSource.h>
#include <USPipeline/DNLFileFrameSource.h>
#include <USPipeline/GstUltrasoundImagePipeline.h>

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
    GstUltrasoundImagePipeline *pipeline = new GstUltrasoundImagePipeline(NULL);
    pipeline->setFrameSource(frame_source);

    pipeline->start();

    std::string tmp;
    std::getline(std::cin, tmp);

    pipeline->stop();

    delete pipeline;
    delete frame_source;

    return 0;
}
