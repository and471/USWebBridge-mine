#ifndef DNLIMAGEEXTRACTOR_H
#define DNLIMAGEEXTRACTOR_H

#include <Modules/USStreamingCommon/DNLImage.h>
#include <USPipelineInterface/PatientMetadata.h>

class DNLImageExtractor
{
public:
    DNLImageExtractor();
    void getPNG(DNLImage::Pointer image, char** data, size_t* size);
    PatientMetadata getPatientMetadata(DNLImage::Pointer image);

    virtual void setLayer(int layer);
protected:
    int layer;
};

#endif // DNLIMAGEEXTRACTOR_H
