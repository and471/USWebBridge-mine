#include "UltrasoundImagePipeline.h"

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include "DNLImageSource.h"
#include "DNLImageExtractor.h"
#include "DNLFrameExchange.h"
#include <Modules/USStreamingCommon/DNLImage.h>

static void onAppSrcNeedDataWrapper(GstAppSrc* appsrc, guint size, gpointer data);
void onAppSrcNeedDataWrapper(GstAppSrc* appsrc, guint size, gpointer data) {
    UltrasoundImagePipeline* pipeline = (UltrasoundImagePipeline*) data;
    pipeline->onAppSrcNeedData(appsrc, size);
}

static void onImageWrapper(DNLImage::Pointer image, void* data);
void onImageWrapper(DNLImage::Pointer image, void* data) {
    UltrasoundImagePipeline* pipeline = (UltrasoundImagePipeline*) data;
    pipeline->onImage(image);
}


UltrasoundImagePipeline::UltrasoundImagePipeline() {
    exchange = new DNLFrameExchange();
    createGstPipeline();
}

void UltrasoundImagePipeline::createGstPipeline() {
    // Create pipeline elements
    pipeline = gst_pipeline_new("pipeline");
    appsrc = gst_element_factory_make("appsrc", "source");
    jpegdec = gst_element_factory_make("pngdec", "j");
    conv = gst_element_factory_make("videoconvert", "conv");
    videoenc = gst_element_factory_make("avenc_mpeg4", "ffenc_mpeg4");
    payloader = gst_element_factory_make("rtpmp4vpay", "rtpmp4vpay");
    udpsink = gst_element_factory_make("udpsink", "udpsink");

    // Set properties
    g_object_set (G_OBJECT (appsrc),
            "stream-type", 0,
            "is-live", TRUE,
            "format", GST_FORMAT_TIME,
            "caps", gst_caps_from_string("image/png"), NULL);
    g_object_set(G_OBJECT(payloader),
            "config-interval", 5, NULL);
    g_object_set(G_OBJECT(udpsink),
            "host", "127.0.0.1",
            "port", 5000, NULL);

    // Callbacks
    g_signal_connect(appsrc, "need-data", G_CALLBACK(onAppSrcNeedDataWrapper), this);

    // Pack
    gst_bin_add_many(GST_BIN (pipeline), appsrc, jpegdec, conv, videoenc, payloader, udpsink, NULL);
    gst_element_link_many(appsrc, jpegdec, conv, videoenc, payloader, udpsink, NULL);
}

void UltrasoundImagePipeline::setDNLImageSource(DNLImageSource* dnl) {
    dnl_image_source = dnl;
    dnl_image_source->connect(&onImageWrapper, this);
}

void UltrasoundImagePipeline::start() {
    dnl_image_source->start();
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void UltrasoundImagePipeline::stop() {
    gst_element_set_state(pipeline, GST_STATE_NULL);
    dnl_image_source->stop();
}

void UltrasoundImagePipeline::onAppSrcNeedData(GstAppSrc* appsrc, guint size) {
    static GstClockTime timestamp = 0;
    GstBuffer *buffer;
    GstFlowReturn ret;

    char* d;
    size_t s;
    DNLImageExtractor::get_png(exchange->get_frame(), &d, &s);

    buffer = gst_buffer_new_allocate (NULL, s, NULL);
    gst_buffer_fill(buffer, 0, (guchar*)d, s);

    free(d);

    GST_BUFFER_PTS (buffer) = timestamp;
    GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 20);
    timestamp += GST_BUFFER_DURATION (buffer);

    ret = gst_app_src_push_buffer(appsrc, buffer);

    if (ret != GST_FLOW_OK) {
        fprintf(stderr, "Gstreamer Error\n");
    }
}

void UltrasoundImagePipeline::onImage(DNLImage::Pointer image) {
    exchange->add_frame(image);
}

