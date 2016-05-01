#include <gstreamermm.h>
#include <gstreamermm/appsrc.h>
#include <gstreamermm/buffer.h>

#include "UltrasoundImagePipeline.h"
#include "DNLImageSource.h"
#include "DNLImageExtractor.h"
#include "DNL2DImageExtractor.h"
#include "DNLFrameExchange.h"
#include <Modules/USStreamingCommon/DNLImage.h>
#include <glib.h>
#include <functional>

/* Wrappers for signals calling instance methods */
/*static void onAppSrcNeedDataWrapper(GstAppSrc* appsrc, guint size, gpointer data);
void onAppSrcNeedDataWrapper(GstAppSrc* appsrc, guint size, gpointer data) {
    UltrasoundImagePipeline* pipeline = (UltrasoundImagePipeline*) data;
    pipeline->onAppSrcNeedData(appsrc);
}*/

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
    //gst_object_unref(pipeline); // frees pipeline and all elements
}

void UltrasoundImagePipeline::createGstPipeline() {
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
    appsrc->property("stream-type", 0);
    appsrc->property("is-live", TRUE);
    appsrc->property("format", Gst::FORMAT_TIME);
    appsrc->property("caps", png_caps);

    // Realtime profile
    videoenc->property("deadline", 1);
    videoenc->property("cpu-used", 4);
    videoenc->property("lag-in-frames", 0);

    //gst_preset_load_preset(GST_PRESET(videoenc), "Profile Realtime");
    udpsink->property<Glib::ustring>("host", "127.0.0.1");
    udpsink->property("port", 5004);

    // Callbacks
    appsrc->signal_need_data().connect(sigc::mem_fun(*this, &UltrasoundImagePipeline::onAppSrcNeedData));

    // Pack
    pipeline->add(appsrc)->add(pngdec)->add(conv)->add(videoenc)->add(payloader)->add(udpsink);
    appsrc->link(pngdec)->link(conv)->link(videoenc)->link(payloader)->link(udpsink);
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
    pipeline->set_state(Gst::STATE_PLAYING);

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

    pipeline->set_state(Gst::STATE_NULL);
    dnl_image_source->stop();
}

void UltrasoundImagePipeline::onAppSrcNeedData(guint _) {
    Frame* frame = exchange->get_frame();

    Glib::RefPtr<Gst::Buffer> buffer = Gst::Buffer::create(frame->getSize());
    Glib::RefPtr<Gst::MapInfo> info(new Gst::MapInfo());
    buffer->map(info, Gst::MAP_WRITE);
    memcpy(info->get_data(), frame->getData(), info->get_size());
    buffer->unmap(info);

    delete frame;

    buffer->set_pts(timestamp);
    buffer->set_duration(Gst::SECOND / getFPS());
    timestamp += buffer->get_duration();

    Gst::FlowReturn val = appsrc->push_buffer(buffer);
    if (val != Gst::FLOW_OK) {
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

    char* data;
    size_t size;
    extractor->getPNG(image, &data, &size);

    Frame* frame = new Frame(data, size);
    free(data);

    exchange->add_frame(frame);
    delete frame;

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

