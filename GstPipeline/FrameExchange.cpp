#include "FrameExchange.h"
#include <USPipelineInterface/Frame.h>
#include <thread>

void FrameExchange::add_frame(Frame* frame) {
    mutex.lock();
    delete this->frame;
    this->frame = Frame::copy(frame);
    mutex.unlock();
}

Frame* FrameExchange::get_frame() {

    // Block until we have at least one frame
    while (frame == nullptr) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    mutex.lock();
    Frame* copy = Frame::copy(frame);
    mutex.unlock();
    return copy;
}

FrameExchange::~FrameExchange() {
    delete frame;
}
