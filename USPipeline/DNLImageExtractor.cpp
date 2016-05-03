#include "DNLImageExtractor.h"
#include <vtkPNGWriter.h>
#include <vtkUnsignedCharArray.h>
#include <cstdio>
#include <vtkImageReslice.h>

DNLImageExtractor::DNLImageExtractor() {
    this->slice = 58;
    this->nSlices = 0;
}

DNLImageExtractor::~DNLImageExtractor() {
    this->pngWriter->Delete();
}

void DNLImageExtractor::getPNG(DNLImage::Pointer image, char** data, size_t* size) {
        vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
        imageData->DeepCopy(image->GetVTKImage());

        // Check if number of slices has changed
        checkNSlicesChanged(imageData);

        vtkMatrix4x4* resliceAxes;
        vtkImageReslice* resampler = vtkImageReslice::New();
        resampler->SetInputData(imageData);

        // Set spacing to be consistent with imageData
        double spacing[3];
        imageData->GetSpacing(spacing);
        resampler->SetOutputSpacing(spacing[0], spacing[0], spacing[0]);

        // Select a slice for 3D imageData
        if (image->GetNDimensions() == 3) {

            // Orientation and position
            double sliceOrientation[16] = {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            };
            double slicePosition[] = {0, 0, slice};

            resliceAxes =  vtkMatrix4x4::New();
            resliceAxes->DeepCopy(sliceOrientation);
            resliceAxes->SetElement(0, 3, slicePosition[0]);
            resliceAxes->SetElement(1, 3, slicePosition[1]);
            resliceAxes->SetElement(2, 3, slicePosition[2]);

            resampler->SetResliceAxes(resliceAxes);
        }

        resampler->SetInterpolationModeToLinear();
        resampler->SetOutputDimensionality(2);

        // Write PNG to memory
        pngWriter->SetWriteToMemory(true);
        pngWriter->SetInputConnection(resampler->GetOutputPort());
        pngWriter->Update();
        pngWriter->Write();

        // Move in memory PNG to new malloced location
        vtkUnsignedCharArray* d = pngWriter->GetResult();
        *size = (size_t)(d->GetSize()*d->GetDataTypeSize());
        *data = (char*) malloc(*size);
        memcpy(*data, (char*)d->GetVoidPointer(0), *size);

        resliceAxes->Delete();
        resampler->Delete();
}

void DNLImageExtractor::checkNSlicesChanged(vtkSmartPointer<vtkImageData> imageData) {
    int extents[6];
    imageData->GetExtent(extents);
    int nSlices = extents[5] - extents[4] + 1;

    if (nSlices != this->nSlices) {
        this->nSlices = nSlices;
        onNSlicesChanged(this->nSlices);
    }
}

void DNLImageExtractor::setOnNSlicesChangedCallback(std::function<void(int)> cb) {
    onNSlicesChangedCallback = cb;
}

void DNLImageExtractor::onNSlicesChanged(int nSlices) {
    onNSlicesChangedCallback(nSlices);
}

void DNLImageExtractor::setSlice(int slice) {
    if (fabs(slice) < nSlices/2.) {
        this->slice = slice;
    }
}


PatientMetadata DNLImageExtractor::getPatientMetadata(DNLImage::Pointer image) {
    PatientMetadata patient;
    patient.name = image->patientName();


    //TODO:remove
    patient.name = "Joe Blo";
    return patient;
}
