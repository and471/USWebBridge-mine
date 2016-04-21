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
    DNLImageSource();
    ~DNLImageSource();

    /// Define signals
    typedef  boost::signals2::signal<void (DNLImage::Pointer )> DNLImageSignalType;
    typedef DNLImageSignalType::slot_type DNLImageHandlerSlotType;

    void connect(DNLImageHandlerSlotType slot);
    virtual void start() = 0;
    virtual void stop() = 0;

protected:
    DNLImageSignalType signal;


};

#endif /// DNLIMAGESOURCE_H_
