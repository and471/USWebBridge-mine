#include <cstdio>
#include <USPipeline/UltrasoundImagePipeline.h>
#include <USPipeline/DNLImageSource.h>
#include <USPipeline/DNLFileImageSource.h>

extern "C" void yo();
void yo() {

    int argc = 0;

    gst_init(&argc, NULL);
    std::string folder = "/home/andrew/Project/forAndrew2D";


    DNLImageSource* dnl_image_source = new DNLFileImageSource(folder);
    UltrasoundImagePipeline *pipeline = new UltrasoundImagePipeline();
    pipeline->setDNLImageSource(dnl_image_source);
    pipeline->start();
}
