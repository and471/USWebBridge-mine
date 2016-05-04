#include <gstreamermm.h>
#include <gstreamermm/appsrc.h>
#include <gstreamermm/buffer.h>

#include "GstUltrasoundImagePipeline.h"
#include "DNLImageSource.h"
#include "DNLImageExtractor.h"
#include "DNLFrameExchange.h"
#include <Modules/USStreamingCommon/DNLImage.h>
#include <glib.h>
#include <functional>

GstUltrasoundImagePipeline::GstUltrasoundImagePipeline(UltrasoundPlugin* plugin) {
    this->plugin = plugin;

    thread = nullptr;
    exchange = new FrameExchange();
    extractor = new DNLImageExtractor();
    extractor->setOnNSlicesChangedCallback(
        std::bind(&GstUltrasoundImagePipeline::onNSlicesChanged, this, std::placeholders::_1)
    );

    plugin->setOnSetSliceCallback(
        std::bind(&GstUltrasoundImagePipeline::onSetSlice, this, std::placeholders::_1)
    );

    createGstPipeline();
}

GstUltrasoundImagePipeline::~GstUltrasoundImagePipeline() {
    delete exchange;
    delete extractor;
    //gst_object_unref(pipeline); // frees pipeline and all elements
}

void GstUltrasoundImagePipeline::createGstPipeline() {
    // Create pipeline elements
    pipeline = Gst::Pipeline::create();
    appsrc = Gst::AppSrc::create();
    pngdec = Gst::ElementFactory::create_element("pngdec");
    conv = Gst::ElementFactory::create_element("videoconvert");
    videoenc = Gst::ElementFactory::create_element("vp8enc");
    payloader = Gst::ElementFactory::create_element("rtpvp8pay");
    udpsink = Gst::ElementFactory::create_element("udpsink");

    // Set properties
    Glib::RefPtr<Gst::Caps> png_caps = Gst::Caps::create_simple("image/png");
    appsrc->property("stream-type", Gst::APP_STREAM_TYPE_STREAM );
    appsrc->property("is-live", TRUE);
    appsrc->property("format", Gst::FORMAT_TIME);
    appsrc->property("caps", png_caps);
    appsrc->property("size", -1);
    appsrc->property("max-bytes", 10000);
    appsrc->property("block", true);

    // Realtime profile
    videoenc->property("deadline", 1);
    videoenc->property("cpu-used", 4);
    videoenc->property("lag-in-frames", 0);

    //gst_preset_load_preset(GST_PRESET(videoenc), "Profile Realtime");
    udpsink->property<Glib::ustring>("host", "127.0.0.1");
    udpsink->property("port", 5004);

    // Callbacks
    appsrc->signal_need_data().connect(sigc::mem_fun(*this, &GstUltrasoundImagePipeline::onAppSrcNeedData));
    appsrc->signal_enough_data().connect(sigc::mem_fun(*this, &GstUltrasoundImagePipeline::onEnough));


    // Pack
    pipeline->add(appsrc)->add(pngdec)->add(conv)->add(videoenc)->add(payloader)->add(udpsink);
    appsrc->link(pngdec)->link(conv)->link(videoenc)->link(payloader)->link(udpsink);
}

void GstUltrasoundImagePipeline::setDNLImageSource(DNLImageSource* dnl) {
    dnl_image_source = dnl;
    dnl_image_source->setOnImageCallback(
        std::bind(&GstUltrasoundImagePipeline::onImage, this, std::placeholders::_1)
    );
}

void GstUltrasoundImagePipeline::start() {
    if (thread != nullptr) {
        fprintf(stderr, "Cannot start new thread: thread is already running\n");
    }
    dnl_image_source->start();
    pipeline->set_state(Gst::STATE_PLAYING);

    running = true;
    thread = new std::thread(&GstUltrasoundImagePipeline::startThread, this);
}

void GstUltrasoundImagePipeline::startThread() {
    while(running) {}
}

void GstUltrasoundImagePipeline::stop() {
    running = false;

    if (thread->joinable()){
        thread->join();
    }
    delete thread;

    pipeline->set_state(Gst::STATE_NULL);
    dnl_image_source->stop();
}

void GstUltrasoundImagePipeline::onAppSrcNeedData(guint _) {
    Frame* frame = exchange->get_frame();

    int queued = appsrc->property_current_level_bytes();

    Glib::RefPtr<Gst::Buffer> buffer = Gst::Buffer::create(frame->getSize());
    Glib::RefPtr<Gst::MapInfo> info(new Gst::MapInfo());
    buffer->map(info, Gst::MAP_WRITE);
    memcpy(info->get_data(), frame->getData(), info->get_size());
    buffer->unmap(info);


    buffer->set_pts(timestamp);
    buffer->set_duration(Gst::SECOND / getFPS());
    timestamp += buffer->get_duration();

    Gst::FlowReturn val = appsrc->push_buffer(buffer);
    if (val != Gst::FLOW_OK) {
        fprintf(stderr, "Gstreamer Error\n");
    }

    delete frame;
}

void GstUltrasoundImagePipeline::onImage(DNLImage::Pointer image) {

    char* data;
    size_t size;
    int slices;
    extractor->getPNG(image, &data, &size);

    Frame* frame = new Frame(data, size);
    free(data);

    exchange->add_frame(frame);
    delete frame;

    // If patient metadata changes, send new metadata
    PatientMetadata patient = extractor->getPatientMetadata(image);
    if (!(patient == this->patient)) {
        this->patient = patient;
        this->plugin->onNewPatientMetadata(patient);
    }
}

void GstUltrasoundImagePipeline::onNSlicesChanged(int nSlices) {
    this->plugin->onNSlicesChanged(nSlices);
}

void GstUltrasoundImagePipeline::onSetSlice(int slice) {
    this->extractor->setSlice(slice);
}

int GstUltrasoundImagePipeline::getFPS() {
    return fps;
}

void GstUltrasoundImagePipeline::setOnNewPatientMetadataCallback(std::function<void(PatientMetadata)> cb) {
    this->onNewPatientMetadataCallback = cb;
}

void GstUltrasoundImagePipeline::onNewPatientMetadata(PatientMetadata patient) {
    this->onNewPatientMetadataCallback(patient);
}

void GstUltrasoundImagePipeline::onEnough() {
    printf("ENOUUUGH!!!");
}

