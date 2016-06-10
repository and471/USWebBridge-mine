#ifndef DNLFRAMESOURCE_H_
#define DNLFRAMESOURCE_H_

#include <USPipelineInterface/FrameSource.h>
#include <Modules/USStreamingCommon/DNLImage.h>
#include <vtkPNGWriter.h>

class DNLFrameSource : public FrameSource {

public:
    virtual void start() = 0;
    virtual void stop() = 0;

    void setSlice(int slice);
    int getNSlices();

protected:
    DNLFrameSource();
    ~DNLFrameSource();

    void onImage(DNLImage::Pointer image);
    int slice;
    int nSlices;

private:
    void getPNG(DNLImage::Pointer image, char** data, size_t* size);
    PatientMetadata getPatientMetadata(DNLImage::Pointer image);
    ImageMetadata getImageMetadata(DNLImage::Pointer image);
    void checkNSlicesChanged(vtkSmartPointer<vtkImageData> imageData);
    double getSpacing(DNLImage::Pointer image);

    vtkPNGWriter* pngWriter;
};

#endif /// DNLIMAGESOURCE_H_
