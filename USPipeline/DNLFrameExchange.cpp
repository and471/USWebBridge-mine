#include "DNLFrameExchange.h"
#include <Modules/USStreamingCommon/DNLImage.h>
#include <thread>

void DNLFrameExchange::add_frame(Frame* frame) {
    mutex.lock();
    delete this->frame;
    this->frame = Frame::copy(frame);
    mutex.unlock();
}

Frame* DNLFrameExchange::get_frame() {

    // Block until we have at least one frame
    while (frame == nullptr) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    mutex.lock();
    Frame* copy = Frame::copy(frame);
    mutex.unlock();
    return copy;
}

DNLFrameExchange::~DNLFrameExchange() {
    delete frame;
}

Frame::Frame(char* data, size_t size) {
    this->size = size;
    this->data = (char*) malloc(size);
    memcpy(this->data, data, size);
}

Frame::~Frame() {
    free(this->data);
}

Frame* Frame::copy(Frame* frame) {
    Frame* new_frame = new Frame(frame->data, frame->size);
    return new_frame;
}

char* Frame::getData() {
    return data;
}

size_t Frame::getSize() {
    return size;
}
