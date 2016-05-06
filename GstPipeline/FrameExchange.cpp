#include "FrameExchange.h"
#include <USPipelineInterface/Frame.h>
#include <thread>

void FrameExchange::add_frame(Frame* frame) {
    std::unique_lock<std::mutex> lock(mutex);
    // LOCK
    delete this->frame;
    this->frame = Frame::copy(frame);
    // UNLOCK
}

Frame* FrameExchange::get_frame() {
    // Block until we have at least one frame
    while (frame == nullptr) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::unique_lock<std::mutex> lock(mutex);
    // LOCK
    Frame* copy = Frame::copy(frame);
    return copy;
    // UNLOCK
}

FrameExchange::~FrameExchange() {
    delete frame;
}
