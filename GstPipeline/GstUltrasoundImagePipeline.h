#ifndef GSTULTRASOUNDIMAGEPIPELINE_H
#define GSTULTRASOUNDIMAGEPIPELINE_H

#include <gstreamermm.h>
#include <gstreamermm/appsrc.h>
#include <glibmm.h>
#include <functional>
#include <thread>
#include "FrameExchange.h"
#include <USPipelineInterface/UltrasoundImagePipeline.h>
#include <USPipelineInterface/UltrasoundController.h>
#include <USPipelineInterface/PatientMetadata.h>
#include <USPipelineInterface/ImageMetadata.h>
#include <USPipelineInterface/FrameSource.h>

class GstUltrasoundImagePipeline : public UltrasoundImagePipeline
{
public:
    GstUltrasoundImagePipeline(UltrasoundController* controller);
    ~GstUltrasoundImagePipeline();

    static void initGst();

    void onAppSrcNeedData(guint size);
    void onNSlicesChanged(int nSlices);

    void onEnough();
    static int getFreePort();

    // Virtual methods
    void start();
    void stop();
    void onSetSlice(int slice);
    void setFrameSource(FrameSource* frame_source);
    void onFrame(Frame* frame);
    int getPort();
    void crop(int left, int right, int top, int bottom);

    void setOnNewPatientMetadataCallback(std::function<void(PatientMetadata)> cb);
    void setOnNewImageMetadataCallback(std::function<void(ImageMetadata)> cb);

    void setFPS(int fps);
    void getQPBounds(int* min, int* max);
    void setBitrate(int bitrate);


private:
    FrameSource* frame_source;
    FrameExchange* exchange;

    bool running = false;
    PatientMetadata patient;
    ImageMetadata metadata;
    int port;

    int fps;
    int qp;

    std::thread* thread;
    Gst::ClockTime timestamp = 0;
    Glib::RefPtr<Gst::Pipeline> pipeline;
    Glib::RefPtr<Gst::AppSrc> appsrc;
    Glib::RefPtr<Gst::Element> pngdec, conv, videocrop, payloader, udpsink, videoenc;

    int onImageCallbackID;
    int onNSlicesChangedCallbackID;

    void startThread();
    int getFPS();
    void createGstPipeline();
    void onNewPatientMetadata(PatientMetadata patient);
    void onNewImageMetadata(ImageMetadata patient);
};

#endif // GSTULTRASOUNDIMAGEPIPELINE_H
