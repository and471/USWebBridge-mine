#include "DNLFrameExchange.h"
#include <Modules/USStreamingCommon/DNLImage.h>
#include <thread>

void DNLFrameExchange::add_frame(DNLImage::Pointer frame) {
    mutex.lock();
    current_frame = frame;
    mutex.unlock();
}

DNLImage::Pointer DNLFrameExchange::get_frame() {
    // Block until we have at least one frame
    while (current_frame == nullptr) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

    }

    return current_frame;
}

