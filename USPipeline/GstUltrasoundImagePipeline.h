#ifndef GSTULTRASOUNDIMAGEPIPELINE_H
#define GSTULTRASOUNDIMAGEPIPELINE_H

#include <gstreamermm.h>
#include <gstreamermm/appsrc.h>
#include <glibmm.h>
#include <functional>

#include "DNLImageSource.h"
#include "DNLImageExtractor.h"
#include "DNLFrameExchange.h"
#include <USPipelineInterface/UltrasoundImagePipeline.h>
#include <USPipelineInterface/UltrasoundPlugin.h>
#include <USPipelineInterface/PatientMetadata.h>

class GstUltrasoundImagePipeline : public UltrasoundImagePipeline
{
public:
    GstUltrasoundImagePipeline(UltrasoundPlugin* plugin);
    ~GstUltrasoundImagePipeline();

    void setDNLImageSource(DNLImageSource *dnl);
    void onAppSrcNeedData(guint size);
    void onImage(DNLImage::Pointer image);
    void onNSlicesChanged(int nSlices);

    void onEnough();

    // Virtual methods
    void start();
    void stop();
    void onSetSlice(int slice);

    void setOnNewPatientMetadataCallback(std::function<void(PatientMetadata)> cb);
    void onNewPatientMetadata(PatientMetadata patient);

private:
    DNLImageExtractor* extractor;
    std::thread* thread;
    Gst::ClockTime timestamp = 0;
    Glib::RefPtr<Gst::Pipeline> pipeline;
    Glib::RefPtr<Gst::AppSrc> appsrc;
    Glib::RefPtr<Gst::Element> pngdec, conv, payloader, udpsink, videoenc;
    FrameExchange* exchange;
    DNLImageSource* dnl_image_source;
    bool running = false;

    PatientMetadata patient;

    void startThread();
    int getFPS();
    void createGstPipeline();
};

#endif // GSTULTRASOUNDIMAGEPIPELINE_H
