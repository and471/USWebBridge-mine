#ifndef ULTRASOUNDIMAGEPIPELINE_H
#define ULTRASOUNDIMAGEPIPELINE_H

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <glib.h>

#include "DNLImageSource.h"
#include "DNLImageExtractor.h"
#include "DNLFrameExchange.h"
#include <USPipelineInterface/interface.h>

class UltrasoundImagePipeline
{
public:
    UltrasoundImagePipeline(USPipelineInterface* interface);
    ~UltrasoundImagePipeline();

    void setDNLImageSource(DNLImageSource *dnl);
    void start();
    void stop();

    void onAppSrcNeedData(GstAppSrc *appsrc, guint size);
    void onImage(DNLImage::Pointer image);

private:
    DNLImageExtractor* extractor;
    USPipelineInterface* interface;
    std::thread* thread;
    GMainLoop* loop;
    GstElement *pipeline, *appsrc, *pngdec, *conv, *payloader, *udpsink, *videoenc;
    DNLFrameExchange* exchange;
    DNLImageSource* dnl_image_source;
    int fps = 20;
    bool running = false;

    PatientMetadata patient;

    void startThread();
    int getFPS();
    void createGstPipeline();
};

#endif // ULTRASOUNDIMAGEPIPELINE_H
