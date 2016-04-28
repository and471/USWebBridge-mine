#include "DNLFrameExchange.h"
#include <Modules/USStreamingCommon/DNLImage.h>
#include <thread>

void DNLFrameExchange::add_frame(DNLImage::Pointer frame) {
    mutex.lock();
    current_frame = frame;

    // TODO: remove
    frame->m_patientName = "Patient Name is Frank";
    mutex.unlock();
}

DNLImage::Pointer DNLFrameExchange::get_frame() {

    // Block until we have at least one frame
    while (current_frame == nullptr) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    mutex.lock();
    DNLImage::Pointer frame = current_frame;
    mutex.unlock();
    return frame;
}

