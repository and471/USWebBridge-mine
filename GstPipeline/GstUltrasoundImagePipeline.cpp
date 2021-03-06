#include <gstreamermm.h>
#include <gstreamermm/appsrc.h>
#include <gstreamermm/buffer.h>

#include "GstUltrasoundImagePipeline.h"
#include "FrameExchange.h"
#include "StreamAdaptor.h"
#include <USPipelineInterface/FrameSource.h>
#include <glib.h>
#include <functional>
#include <cstring>
#include <mutex>

#include <math.h>
#include <sys/time.h>


static bool initialised = false;
static std::mutex creationMutex;
static int portCounter = 0;

GstUltrasoundImagePipeline::GstUltrasoundImagePipeline(UltrasoundController* controller) {
    // Seem to be issues with concurrently initialising pipelines, so prevent this
    std::unique_lock<std::mutex> lock(creationMutex);

    if (!initialised) initGst();

    this->controller = controller;

    thread = nullptr;
    exchange = new FrameExchange();
    streamAdaptor = new StreamAdaptor();

    controller->setOnSetSliceCallback(
        std::bind(&GstUltrasoundImagePipeline::onSetSlice, this, std::placeholders::_1)
    );

    fps = 8;

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
    videocrop = Gst::ElementFactory::create_element("videocrop");
    videoenc = Gst::ElementFactory::create_element("vp8enc");
    payloader = Gst::ElementFactory::create_element("rtpvp8pay");
    udpsink = Gst::ElementFactory::create_element("udpsink");

    // Set properties
    Glib::RefPtr<Gst::Caps> png_caps = Gst::Caps::create_simple("image/png");
    appsrc->property("stream-type", Gst::APP_STREAM_TYPE_STREAM);
    appsrc->property("is-live", TRUE);
    appsrc->property("format", Gst::FORMAT_TIME);
    appsrc->property("caps", png_caps);
    appsrc->property("size", -1);
    appsrc->property("max-bytes", 10000);
    appsrc->property("block", true);

    // Realtime profile
    videoenc->property("deadline", 1);
    videoenc->property("cpu-used", 10);
    videoenc->property("lag-in-frames", 0);
    videoenc->property("error-resilient", 1);
    videoenc->property("keyframe-max-dist", 8);
    videoenc->property("static-threshold", 800);

    videoenc->property("min-quantizer", 8);
    videoenc->property("max-quantizer", 40);

    videoenc->property("target-bitrate", 1200*1000);

    payloader->property("mtu", 1024);

    udpsink->property<Glib::ustring>("host", "127.0.0.1");
    udpsink->property("port", port);

    // Callbacks
    appsrc->signal_need_data().connect(sigc::mem_fun(*this, &GstUltrasoundImagePipeline::onAppSrcNeedData));

    // Pack
    pipeline->add(appsrc)->add(pngdec)->add(conv)->add(videocrop)->add(videoenc)->add(payloader)->add(udpsink);
    appsrc->link(pngdec)->link(conv)->link(videocrop)->link(videoenc)->link(payloader)->link(udpsink);

}

int GstUltrasoundImagePipeline::getFreePort() {
    return 5005 + (portCounter++);
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
    startTime = getMilliseconds();

    if (thread != nullptr) {
        fprintf(stderr, "Cannot start new thread: thread is already running\n");
    }
    pipeline->set_state(Gst::STATE_PLAYING);

    running = true;
    thread = new std::thread(&GstUltrasoundImagePipeline::startThread, this);

    // Manually fire these events when we start
    onNSlicesChanged(frame_source->getNSlices());
    onNewPatientMetadata(patient);
}

void GstUltrasoundImagePipeline::startThread() {
    while(running) {}
}

void GstUltrasoundImagePipeline::stop() {
    running = false;

    if (thread && thread->joinable()){
        thread->join();
    }
    delete thread;

    pipeline->set_state(Gst::STATE_NULL);
}

void GstUltrasoundImagePipeline::onAppSrcNeedData(guint _) {

    Frame* frame = exchange->get_frame();

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
        onNewPatientMetadata(patient);
    }

    // If image metadata changes, send new metadata
    ImageMetadata metadata = frame->getImageMetadata();
    if (!(metadata == this->metadata)) {
        this->metadata = metadata;
        onNewImageMetadata(metadata);
    }

}

void GstUltrasoundImagePipeline::crop(int left, int right, int top, int bottom) {
    int width, height;
    Glib::RefPtr<Gst::Caps> caps = conv->get_static_pad("src")->get_current_caps();
    caps->get_structure(0).get_field("width", width);
    caps->get_structure(0).get_field("height", height);

    if (left == 0 && right == 0 && top == 0 && bottom == 0 ||
        left == right || top == bottom)
    {
        // Reset & also don't let a zero sized stream be sent
        videocrop->property("top", 0);
        videocrop->property("left", 0);
        videocrop->property("right", 0);
        videocrop->property("bottom", 0);
    } else {
        videocrop->property("top", top);
        videocrop->property("left", left);
        videocrop->property("right", width-right);
        videocrop->property("bottom", height-bottom);
    }
}

void GstUltrasoundImagePipeline::onNSlicesChanged(int nSlices) {
    if (nSlices > 1) {
        streamAdaptor->setProfile(_3D);
    } else {
        streamAdaptor->setProfile(_2D);
    }

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

void GstUltrasoundImagePipeline::setOnNewImageMetadataCallback(std::function<void(ImageMetadata)> cb) {
    this->onNewImageMetadataCallback = cb;
}

void GstUltrasoundImagePipeline::onNewImageMetadata(ImageMetadata metadata) {
    this->onNewImageMetadataCallback(metadata);
}

void GstUltrasoundImagePipeline::setFPS(int fps) {
    this->fps = fps;
}

void GstUltrasoundImagePipeline::setBitrate(int bitrate) {
    if (!videoenc) return;
    if (bitrate < 1000) return;

    double error = 0.1;

    int FPS = streamAdaptor->getFPS(bitrate);
    if (FPS != -1) {
        setFPS(FPS);
    }

    bitrate = bitrate * (1-error);
    videoenc->property("target-bitrate", bitrate);
}
