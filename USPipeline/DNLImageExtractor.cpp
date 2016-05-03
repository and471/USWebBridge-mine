#include "DNLImageExtractor.h"
#include <vtkPNGWriter.h>
#include <vtkUnsignedCharArray.h>
#include <cstdio>
#include <vtkImageReslice.h>

DNLImageExtractor::DNLImageExtractor() {
    this->slice = 0;
    this->nSlices = 0;
}

void DNLImageExtractor::getPNG(DNLImage::Pointer image, char** data, size_t* size) {
        vtkSmartPointer<vtkImageData> imageData = image->GetVTKImage();

        // Check if number of slices has changed
        checkNSlicesChanged(imageData);

        vtkSmartPointer<vtkImageReslice> resampler = vtkSmartPointer<vtkImageReslice>::New();
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

            vtkSmartPointer<vtkMatrix4x4> resliceAxes =  vtkSmartPointer<vtkMatrix4x4>::New();
            resliceAxes->DeepCopy(sliceOrientation);
            resliceAxes->SetElement(0, 3, slicePosition[0]);
            resliceAxes->SetElement(1, 3, slicePosition[1]);
            resliceAxes->SetElement(2, 3, slicePosition[2]);

            resampler->SetResliceAxes(resliceAxes);
        }

        resampler->SetInterpolationModeToLinear();
        resampler->SetOutputDimensionality(2);

        // Write PNG to memory
        vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
        writer->SetWriteToMemory(true);
        writer->SetInputConnection(resampler->GetOutputPort());
        writer->Update();
        writer->Write();

        // Move in memory PNG to new malloced location
        vtkSmartPointer<vtkUnsignedCharArray> d = writer->GetResult();
        *size = (size_t)(d->GetSize()*d->GetDataTypeSize());
        *data = (char*) malloc(*size);
        memcpy(*data, (char*)d->GetVoidPointer(0), *size);
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
