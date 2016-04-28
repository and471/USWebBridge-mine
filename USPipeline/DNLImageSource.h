#ifndef DNLIMAGESOURCE_H_
#define DNLIMAGESOURCE_H_

#include <string>
#include <vector>
#include <boost/filesystem/operations.hpp>
#include <sstream>
#include <thread>
#include <boost/signals2.hpp>
#include <Modules/USStreamingCommon/DNLImage.h>
#include <functional>

class DNLImageSource {

public:

    void setOnImageCallback(std::function<void(DNLImage::Pointer)> cb);
    void onImage(DNLImage::Pointer image);

    virtual void start() = 0;
    virtual void stop() = 0;

private:

    std::function<void(DNLImage::Pointer)> onImageCallback;

};

#endif /// DNLIMAGESOURCE_H_
