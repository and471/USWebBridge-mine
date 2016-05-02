#ifndef GSTULTRASOUNDIMAGEPIPELINE_H
#define GSTULTRASOUNDIMAGEPIPELINE_H

#include <gstreamermm.h>
#include <gstreamermm/appsrc.h>
#include <glibmm.h>

#include "DNLImageSource.h"
#include "DNLImageExtractor.h"
#include "DNLFrameExchange.h"
#include <USPipelineInterface/UltrasoundImagePipeline.h>
#include <USPipelineInterface/UltrasoundPlugin.h>

class GstUltrasoundImagePipeline : public UltrasoundImagePipeline
{
public:
    GstUltrasoundImagePipeline(USPipelineInterface* interface);
    ~GstUltrasoundImagePipeline();

    void setDNLImageSource(DNLImageSource *dnl);
    void onAppSrcNeedData(guint size);
    void onImage(DNLImage::Pointer image);


    void start();
    void stop();
    void onNSlicesChanged(int nSlices);
    void onSetSlice(int slice);

private:
    DNLImageExtractor* extractor;
    USPipelineInterface* interface;
    std::thread* thread;
    Gst::ClockTime timestamp = 0;
    Glib::RefPtr<Gst::Pipeline> pipeline;
    Glib::RefPtr<Gst::AppSrc> appsrc;
    Glib::RefPtr<Gst::Element> pngdec, conv, payloader, udpsink, videoenc;
    DNLFrameExchange* exchange;
    DNLImageSource* dnl_image_source;
    bool running = false;

    PatientMetadata patient;

    void startThread();
    int getFPS();
    void createGstPipeline();
};

#endif // GSTULTRASOUNDIMAGEPIPELINE_H
