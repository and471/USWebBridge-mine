#ifndef DNLIMAGEEXTRACTOR_H
#define DNLIMAGEEXTRACTOR_H

#include <Modules/USStreamingCommon/DNLImage.h>

class DNLImageExtractor
{
public:
    static void get_png(DNLImage::Pointer image, char** data, size_t* size);
private:
    DNLImageExtractor();
};

#endif // DNLIMAGEEXTRACTOR_H
