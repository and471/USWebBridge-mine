#include <cstdio>
#include <functional>
#include <USPipeline/GstUltrasoundImagePipeline.h>
#include <USPipeline/DNLImageSource.h>
#include <USPipeline/DNLFileImageSource.h>
#include "UltrasoundPlugin.h"
#include <gstreamermm.h>

#include "PatientMetadata.h"
#include "interface.h"

UltrasoundImagePipeline* initGstUltrasoundImagePipelineJanusPlugin(UltrasoundPlugin* plugin) {
    int z = 0;
    int &argc = z;

    char** h = NULL;
    char** &argv = h;

    Gst::init(argc, argv);

    std::string folder = "/home/andrew/Project/forAndrew3D";

    DNLImageSource* dnl_image_source = new DNLFileImageSource(folder);
    GstUltrasoundImagePipeline* pipeline = new GstUltrasoundImagePipeline(plugin);
    pipeline->setDNLImageSource(dnl_image_source);

    return pipeline;
}
