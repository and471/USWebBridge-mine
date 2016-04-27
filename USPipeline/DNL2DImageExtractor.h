#ifndef DNL2DIMAGEEXTRACTOR_H
#define DNL2DIMAGEEXTRACTOR_H

#include <Modules/USStreamingCommon/DNLImage.h>
#include "DNLImageExtractor.h"

class DNL2DImageExtractor : public DNLImageExtractor {

public:
    DNL2DImageExtractor();
    void setLayer(int layer);
};

#endif // DNL2DIMAGEEXTRACTOR_H
