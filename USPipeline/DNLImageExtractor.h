#ifndef DNLIMAGEEXTRACTOR_H
#define DNLIMAGEEXTRACTOR_H

#include <Modules/USStreamingCommon/DNLImage.h>
#include <USPipelineInterface/PatientMetadata.h>
#include <functional>
#include <vtkPNGWriter.h>

class DNLImageExtractor
{
public:
    DNLImageExtractor();
    void getPNG(DNLImage::Pointer image, char** data, size_t* size);
    PatientMetadata getPatientMetadata(DNLImage::Pointer image);

    void checkNSlicesChanged(vtkSmartPointer<vtkImageData> imageData);
    void setSlice(int slice);

    void onNSlicesChanged(int nSlices);
    void setOnNSlicesChangedCallback(std::function<void(int)> cb);


private:
    int slice;
    int nSlices;
    std::function<void(int)> onNSlicesChangedCallback;
    vtkPNGWriter* pngWriter = vtkPNGWriter::New();
};

#endif // DNLIMAGEEXTRACTOR_H
