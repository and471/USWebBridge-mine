#include "DNLImageSource.h"
#include <map>
#include <ctime>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <Modules/USStreamingCommon/DNLImageReader.h>
#include <functional>


void DNLImageSource::setOnImageCallback(std::function<void(DNLImage::Pointer)> cb) {
    onImageCallback = cb;
}

void DNLImageSource::onImage(DNLImage::Pointer image) {
    onImageCallback(image);
}
