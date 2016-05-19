#include <gstreamermm.h>
#include <gstreamermm/appsrc.h>
#include <gstreamermm/buffer.h>

#include "GstUltrasoundImagePipeline.h"
#include "FrameExchange.h"
#include <USPipelineInterface/FrameSource.h>
#include <glib.h>
#include <functional>
#include <cstring>

static bool initialised = false;
static int portCounter = 0;

GstUltrasoundImagePipeline::GstUltrasoundImagePipeline(UltrasoundController* controller) {
    if (!initialised) initGst();

    this->controller = controller;

    thread = nullptr;
    exchange = new FrameExchange();

    controller->setOnSetSliceCallback(
        std::bind(&GstUltrasoundImagePipeline::onSetSlice, this, std::placeholders::_1)
    );

    fps = 40;

    port = getFreePort();
    createGstPipeline();
}

void GstUltrasoundImagePipeline::initGst() {
    initialised = true;

    int z = 0;
    int &argc = z;

    char** h = NULL;
    char** &argv = h;

    Gst::init(argc, argv);
}

GstUltrasoundImagePipeline::~GstUltrasoundImagePipeline() {
    frame_source->removeOnFrameCallback(onImageCallbackID);
    frame_source->removeOnNSlicesChangedCallback(onNSlicesChangedCallbackID);
    delete exchange;
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
    //videoenc->property("keyframe-max-dist", 128);

    //gst_preset_load_preset(GST_PRESET(videoenc), "Profile Realtime");
    udpsink->property<Glib::ustring>("host", "127.0.0.1");
    udpsink->property("port", port);

    // Callbacks
    appsrc->signal_need_data().connect(sigc::mem_fun(*this, &GstUltrasoundImagePipeline::onAppSrcNeedData));

    // Pack
    pipeline->add(appsrc)->add(pngdec)->add(conv)->add(videoenc)->add(payloader)->add(udpsink);
    appsrc->link(pngdec)->link(conv)->link(videoenc)->link(payloader)->link(udpsink);

    GST_DEBUG_BIN_TO_DOT_FILE((GstBin*)pipeline->gobj(), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
}

int GstUltrasoundImagePipeline::getFreePort() {
    return 5004 + (portCounter++);
}

int GstUltrasoundImagePipeline::getPort() {
    return port;
}

void GstUltrasoundImagePipeline::setFrameSource(FrameSource* frame_source) {
    this->frame_source = frame_source;
    onImageCallbackID = frame_source->addOnFrameCallback(
        std::bind(&GstUltrasoundImagePipeline::onFrame, this, std::placeholders::_1)
    );
    onNSlicesChangedCallbackID = frame_source->addOnNSlicesChangedCallback(
        std::bind(&GstUltrasoundImagePipeline::onNSlicesChanged, this, std::placeholders::_1)
    );
}

void GstUltrasoundImagePipeline::start() {
    if (thread != nullptr) {
        fprintf(stderr, "Cannot start new thread: thread is already running\n");
    }
    frame_source->start();
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
    frame_source->stop();
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

void GstUltrasoundImagePipeline::onFrame(Frame* frame) {
    exchange->add_frame(frame);

    // If patient metadata changes, send new metadata
    PatientMetadata patient = frame->getPatientMetadata();
    if (!(patient == this->patient)) {
        this->patient = patient;
        this->controller->onNewPatientMetadata(patient);
    }
}

void GstUltrasoundImagePipeline::onNSlicesChanged(int nSlices) {
    controller->onNSlicesChanged(nSlices);
}

void GstUltrasoundImagePipeline::onSetSlice(int slice) {
    frame_source->setSlice(slice);
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

