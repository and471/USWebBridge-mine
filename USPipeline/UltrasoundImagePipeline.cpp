#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include "UltrasoundImagePipeline.h"
#include "DNLImageSource.h"
#include "DNLImageExtractor.h"
#include "DNL2DImageExtractor.h"
#include "DNLFrameExchange.h"
#include <Modules/USStreamingCommon/DNLImage.h>
#include <glib.h>
#include <functional>

/* Wrappers for signals calling instance methods */
static void onAppSrcNeedDataWrapper(GstAppSrc* appsrc, guint size, gpointer data);
void onAppSrcNeedDataWrapper(GstAppSrc* appsrc, guint size, gpointer data) {
    UltrasoundImagePipeline* pipeline = (UltrasoundImagePipeline*) data;
    pipeline->onAppSrcNeedData(appsrc);
}

UltrasoundImagePipeline::UltrasoundImagePipeline(USPipelineInterface* interface) {
    this->interface = interface;

    extractor = nullptr;
    thread = nullptr;
    exchange = new DNLFrameExchange();
    createGstPipeline();
}

UltrasoundImagePipeline::~UltrasoundImagePipeline() {
    delete exchange;
    delete extractor;
    gst_object_unref(pipeline); // frees pipeline and all elements
}

void UltrasoundImagePipeline::createGstPipeline() {
    // Create pipeline elements
    pipeline = gst_pipeline_new("pipeline");
    appsrc = gst_element_factory_make("appsrc", "source");
    pngdec = gst_element_factory_make("pngdec", "j");
    conv = gst_element_factory_make("videoconvert", "conv");
    videoenc = gst_element_factory_make("vp8enc", "ffenc_mpeg4");
    payloader = gst_element_factory_make("rtpvp8pay", "rtpmp4vpay");
    udpsink = gst_element_factory_make("udpsink", "udpsink");

    // Set properties
    GstCaps* png_caps = gst_caps_from_string("image/png");
    g_object_set(G_OBJECT (appsrc),
            "stream-type", 0,
            "is-live", TRUE,
            "format", GST_FORMAT_TIME,
            "caps", png_caps, NULL);
    gst_caps_unref(png_caps);

    gst_preset_load_preset(GST_PRESET(videoenc), "Profile Realtime");
    g_object_set(G_OBJECT(udpsink),
            "host", "127.0.0.1",
            "port", 5004, NULL);

    // Callbacks
    g_signal_connect(appsrc, "need-data", G_CALLBACK(onAppSrcNeedDataWrapper), this);

    // Pack
    gst_bin_add_many(GST_BIN (pipeline), appsrc, pngdec, conv, videoenc, payloader, udpsink, NULL);
    gst_element_link_many(appsrc, pngdec, conv, videoenc, payloader, udpsink, NULL);
}

void UltrasoundImagePipeline::setDNLImageSource(DNLImageSource* dnl) {
    dnl_image_source = dnl;
    dnl_image_source->setOnImageCallback(
        std::bind(&UltrasoundImagePipeline::onImage, this, std::placeholders::_1)
    );
}

void UltrasoundImagePipeline::start() {
    if (thread != nullptr) {
        fprintf(stderr, "Cannot start new thread: thread is already running\n");
    }
    dnl_image_source->start();
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    running = true;
    thread = new std::thread(&UltrasoundImagePipeline::startThread, this);
}

void UltrasoundImagePipeline::startThread() {
    while(running) {}
}

void UltrasoundImagePipeline::stop() {
    running = false;

    if (thread->joinable()){
        thread->join();
    }
    delete thread;

    gst_element_set_state(pipeline, GST_STATE_NULL);
    dnl_image_source->stop();
}

void UltrasoundImagePipeline::onAppSrcNeedData(GstAppSrc* appsrc) {
    char* d;
    size_t s;
    extractor->getPNG(exchange->get_frame(), &d, &s);

    GstMapInfo info;
    GstBuffer* buffer = gst_buffer_new_allocate(NULL, s, NULL);
    gst_buffer_map(buffer, &info, GST_MAP_WRITE);
    memcpy(info.data, d, info.size);
    gst_buffer_unmap(buffer, &info);

    free(d);

    GST_BUFFER_PTS(buffer) = timestamp;
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, getFPS());

    timestamp += GST_BUFFER_DURATION(buffer);

    GstFlowReturn ret = gst_app_src_push_buffer(appsrc, buffer);
    if (ret != GST_FLOW_OK) {
        fprintf(stderr, "Gstreamer Error\n");
    }
}

void UltrasoundImagePipeline::onImage(DNLImage::Pointer image) {
    // First time we receive an image, check if 2D or 3D and set up extractor
    if (extractor == nullptr) {
        if (image->GetNDimensions() == 2) {
            extractor = new DNL2DImageExtractor();
        } else {
            extractor = new DNLImageExtractor();
        }
    }

    exchange->add_frame(image);

    // If patient metadata changes, send new metadata
    PatientMetadata patient = extractor->getPatientMetadata(image);
    if (!(patient == this->patient)) {
        this->patient = patient;
        this->interface->OnNewPatientMetadata(patient);
    }
}

int UltrasoundImagePipeline::getFPS() {
    return fps;
}

