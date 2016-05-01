#include <cstdio>
#include <gstreamermm.h>
#include <string>

#include <USPipeline/DNLImageSource.h>
#include <USPipeline/DNLFileImageSource.h>
#include <USPipeline/UltrasoundImagePipeline.h>

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Not enough arguments\n");
        printf("Usage:\n");
        printf("%s <folder>\n", argv[0]);
        return -1;
    }


    Gst::init(argc, argv);
    std::string folder = argv[1];

    DNLImageSource* dnl_image_source = new DNLFileImageSource(folder);
    UltrasoundImagePipeline *pipeline = new UltrasoundImagePipeline(NULL);
    pipeline->setDNLImageSource(dnl_image_source);

    pipeline->start();

    std::string tmp;
    std::getline(std::cin, tmp);

    pipeline->stop();

    delete pipeline;
    delete dnl_image_source;

    return 0;
}
