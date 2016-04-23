#ifndef DNLIMAGESOURCE_H_
#define DNLIMAGESOURCE_H_

#include <string>
#include <vector>
#include <boost/filesystem/operations.hpp>
#include <sstream>
#include <thread>
#include <boost/signals2.hpp>
#include <Modules/USStreamingCommon/DNLImage.h>

class DNLImageSource {

public:

    void connect(void (*handler)(DNLImage::Pointer, void*), void* data);
    void onImage(DNLImage::Pointer image);

    virtual void start() = 0;
    virtual void stop() = 0;

private:

    void (*image_handler)(DNLImage::Pointer, void*) = nullptr;
    void* image_handler_data = nullptr;

};

#endif /// DNLIMAGESOURCE_H_
