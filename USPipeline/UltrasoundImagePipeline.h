#ifndef ULTRASOUNDIMAGEPIPELINE_H
#define ULTRASOUNDIMAGEPIPELINE_H

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include "DNLImageSource.h"
#include "DNLFrameExchange.h"
#include <glib.h>

class UltrasoundImagePipeline
{
public:
    UltrasoundImagePipeline();
    ~UltrasoundImagePipeline();

    void setDNLImageSource(DNLImageSource *dnl);
    void start();
    void stop();

    void onAppSrcNeedData(GstAppSrc *appsrc, guint size);
    void onImage(DNLImage::Pointer image);

private:
    std::thread* thread = nullptr;
    GMainLoop* loop;
    GstElement *pipeline, *appsrc, *pngdec, *conv, *payloader, *udpsink, *videoenc;
    DNLFrameExchange* exchange;
    DNLImageSource* dnl_image_source;
    int fps = 20;

    static void startThread(GMainLoop* loop);
    int getFPS();
    void createGstPipeline();
};

#endif // ULTRASOUNDIMAGEPIPELINE_H
