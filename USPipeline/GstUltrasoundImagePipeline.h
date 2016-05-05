#ifndef GSTULTRASOUNDIMAGEPIPELINE_H
#define GSTULTRASOUNDIMAGEPIPELINE_H

#include <gstreamermm.h>
#include <gstreamermm/appsrc.h>
#include <glibmm.h>
#include <functional>
#include <thread>
#include "FrameExchange.h"
#include <USPipelineInterface/UltrasoundImagePipeline.h>
#include <USPipelineInterface/UltrasoundPlugin.h>
#include <USPipelineInterface/PatientMetadata.h>
#include <USPipelineInterface/FrameSource.h>

class GstUltrasoundImagePipeline : public UltrasoundImagePipeline
{
public:
    GstUltrasoundImagePipeline(UltrasoundController* controller);
    ~GstUltrasoundImagePipeline();

    static void initGst();

    void onAppSrcNeedData(guint size);
    void onFrame(Frame* frame);
    void onNSlicesChanged(int nSlices);

    void onEnough();
    static int getFreePort();

    // Virtual methods
    void start();
    void stop();
    void onSetSlice(int slice);
    void setFrameSource(FrameSource* frame_source);
    int getPort();

    void setOnNewPatientMetadataCallback(std::function<void(PatientMetadata)> cb);
    void onNewPatientMetadata(PatientMetadata patient);

private:
    FrameSource* frame_source;
    FrameExchange* exchange;

    bool running = false;
    PatientMetadata patient;
    int port;

    std::thread* thread;
    Gst::ClockTime timestamp = 0;
    Glib::RefPtr<Gst::Pipeline> pipeline;
    Glib::RefPtr<Gst::AppSrc> appsrc;
    Glib::RefPtr<Gst::Element> pngdec, conv, payloader, udpsink, videoenc;

    int onImageCallbackID;
    int onNSlicesChangedCallbackID;

    void startThread();
    int getFPS();
    void createGstPipeline();
};

#endif // GSTULTRASOUNDIMAGEPIPELINE_H
