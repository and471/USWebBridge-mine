#include "UltrasoundImagePipeline.h"

UltrasoundImagePipeline::UltrasoundImagePipeline()
{


}

UltrasoundImagePipeline::createGstPipeline() {
    // Create pipeline elements
    pipeline = gst_pipeline_new("pipeline");
    appsrc = gst_element_factory_make("appsrc", "source");
    jpegdec = gst_element_factory_make("jpegdec", "j");
    conv = gst_element_factory_make("videoconvert", "conv");
    videoenc = gst_element_factory_make("avenc_mpeg4", "ffenc_mpeg4");
    payloader = gst_element_factory_make("rtpmp4vpay", "rtpmp4vpay");
    udpsink = gst_element_factory_make("udpsink", "udpsink");

    // Set properties
    g_object_set (G_OBJECT (appsrc),
            "stream-type", 0,
            "is-live", TRUE,
            "format", GST_FORMAT_TIME,
            "caps", gst_caps_from_string("image/jpeg"), NULL);
    g_object_set(G_OBJECT(payloader),
            "config-interval", 5, NULL);
    g_object_set(G_OBJECT(udpsink),
            "host", "127.0.0.1",
            "port", 5000, NULL);

    // Callbacks
    GstAppSrcCallbacks appsrc_callbacks;
    appsrc_callbacks.need_data = on_app_src_need_data;
    gst_app_src_set_callbacks(GST_APP_SRC_CAST(appsrc), &appsrc_callbacks, NULL, NULL);

    // Pack
    gst_bin_add_many(GST_BIN (pipeline), appsrc, jpegdec, conv, videoenc, payloader, udpsink, NULL);
    gst_element_link_many(appsrc, jpegdec, conv, videoenc, payloader, udpsink, NULL);
}

UltrasoundImagePipeline::setDNLImageSource(DNLImageSource *dnl) {
    dnl_image_source = dnl;
    dnl_image_source->connect(&on_image_produced);
}

UltrasoundImagePipeline::start() {
    dnl_image_source->start();
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

UltrasoundImagePipeline::stop() {
    gst_element_set_state (pipeline, GST_STATE_NULL);
    dnl_image_source->stop();
}

