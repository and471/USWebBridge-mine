#ifndef DNLIMAGEEXTRACTOR_H
#define DNLIMAGEEXTRACTOR_H

#include <Modules/USStreamingCommon/DNLImage.h>

class DNLImageExtractor
{
public:
    DNLImageExtractor();
    void get_png(DNLImage::Pointer image, char** data, size_t* size);
    virtual void setLayer(int layer);
protected:
    int layer;
};

#endif // DNLIMAGEEXTRACTOR_H
