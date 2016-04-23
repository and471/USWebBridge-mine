#ifndef ULTRASOUNDIMAGEPIPELINE_H
#define ULTRASOUNDIMAGEPIPELINE_H

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include "DNLImageSource.h"
#include "DNLFrameExchange.h"

class UltrasoundImagePipeline
{
public:
    UltrasoundImagePipeline();

    void setDNLImageSource(DNLImageSource *dnl);
    void start();
    void stop();

    void onAppSrcNeedData(GstAppSrc *appsrc, guint size);
    void onImage(DNLImage::Pointer image);

private:
    GstElement *pipeline, *appsrc, *pngdec, *conv, *payloader, *udpsink, *videoenc;
    DNLFrameExchange* exchange;
    DNLImageSource* dnl_image_source;

    void createGstPipeline();
};

#endif // ULTRASOUNDIMAGEPIPELINE_H
