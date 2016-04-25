#include "DNLImageSource.h"
#include <map>
#include <ctime>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <Modules/USStreamingCommon/DNLImageReader.h>

void DNLImageSource::connect(void (*handler)(DNLImage::Pointer, void*), void* data) {
    image_handler = handler;
    image_handler_data = data;
}

void DNLImageSource::onImage(DNLImage::Pointer image) {
    if (image_handler != nullptr) {
        image_handler(image, image_handler_data);
    }
}
