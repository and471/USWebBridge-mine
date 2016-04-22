#include "DNLFrameExchange.h"
#include <Modules/USStreamingCommon/DNLImage.h>

DNLFrameExchange::DNLFrameExchange() {
}

void DNLFrameExchange::add_frame(DNLImage::Pointer frame) {
    mutex.lock();
    current_frame = frame;
    mutex.unlock();
}

DNLImage::Pointer DNLFrameExchange::get_frame() {
    mutex.lock();
    DNLImage::Pointer frame = current_frame;
    mutex.unlock();
    return frame;
}

