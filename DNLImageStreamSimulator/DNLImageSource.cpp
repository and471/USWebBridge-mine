#include "DNLImageSource.h"
#include <map>
#include <ctime>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <Modules/USStreamingCommon/DNLImageReader.h>

DNLImageSource::DNLImageSource() {}
DNLImageSource::~DNLImageSource() {}


void DNLImageSource::connect(DNLImageHandlerSlotType slot) {
    this->signal.connect(slot);
}
