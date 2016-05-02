#ifndef ULTRASOUNDIMAGEPIPELINE_H
#define ULTRASOUNDIMAGEPIPELINE_H

#include <gstreamermm.h>
#include <gstreamermm/appsrc.h>
#include <glibmm.h>

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

    void onAppSrcNeedData(guint size);
    void onImage(DNLImage::Pointer image);
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
    int fps = 20;
    bool running = false;

    PatientMetadata patient;

    void startThread();
    int getFPS();
    void createGstPipeline();
};

#endif // ULTRASOUNDIMAGEPIPELINE_H
