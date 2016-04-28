#include "DNLImageExtractor.h"
#include <vtkPNGWriter.h>
#include <vtkUnsignedCharArray.h>
#include <cstdio>
#include <vtkImageReslice.h>

DNLImageExtractor::DNLImageExtractor() {
    this->layer = 0;
}

void DNLImageExtractor::getPNG(DNLImage::Pointer image, char** data, size_t* size) {
        vtkSmartPointer<vtkImageReslice> resampler = vtkSmartPointer<vtkImageReslice>::New();
        // input data parameters
        double spacing[3];
        image->GetVTKImage()->GetSpacing(spacing);

        resampler->SetInputData(image->GetVTKImage());
        resampler->SetOutputSpacing(spacing[0], spacing[0], spacing[0]); // 0 for example, but could be any other

        //if (image->GetNDimensions() == 2){
            //sliceData = image->GetVTKImage();
        //} else
        if (image->GetNDimensions() == 3){

                static double axialElements[16] = {
                         1, 0, 0, 0,
                         0, 1, 0, 0,
                         0, 0, 1, 0,
                         0, 0, 0, 1 };

            vtkSmartPointer<vtkMatrix4x4> resliceAxes =  vtkSmartPointer<vtkMatrix4x4>::New();
            resliceAxes->DeepCopy(axialElements);

            //resliceAxes->Identity(); // slice on the z plane passing by (0,0,0)

            double point_in_plane[] = {0,0,0};

            resampler->SetResliceAxes(resliceAxes);
            resliceAxes->SetElement(0, 3, point_in_plane[0]);
            resliceAxes->SetElement(1, 3, point_in_plane[1]);
            resliceAxes->SetElement(2, 3, point_in_plane[2]);

            //vtkSmartPointer<vtkExtractVOI> slicer = vtkSmartPointer<vtkExtractVOI>::New();
            //slicer->SetInputData(image->GetVTKImage());
            //int *extent = image->GetVTKImage()->GetExtent();
            //slicer->SetVOI((extent[1]+extent[0])/2, (extent[1]+extent[0])/2, extent[2], extent[3], extent[4], extent[5]);
            //int slice = (extent[5]+extent[4])/2;
            //slicer->SetVOI(extent[0], extent[1], extent[2], extent[3], slice,slice);
            //resampler->SetOutputExtent(extent[0], extent[1], extent[2], extent[3], slice,slice);
            //slicer->Update();
            //sliceData = vtkSmartPointer<vtkImageData>::New();
            //sliceData->DeepCopy(slicer->GetOutput());
        }

        resampler->SetInterpolationModeToLinear();
        resampler->SetOutputDimensionality(2);
        //resampler->ReleaseDataFlagOff();
        //resampler->Update();

        //sliceData = vtkSmartPointer<vtkImageData>::New();
        //sliceData->DeepCopy(resampler->GetOutput());



        vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
        writer->SetWriteToMemory(true);
        //writer->SetFileName("/home/ag09_local/data/ProjectsWithBernhard/phantomData/test.png");
        //writer->SetInputData(sliceData);
        writer->SetInputConnection(resampler->GetOutputPort());
        writer->Update();
        writer->Write();

        // Move JPEG wrote in memory to new malloced location
        vtkSmartPointer<vtkUnsignedCharArray> d = writer->GetResult();
        *size = (size_t)(d->GetSize()*d->GetDataTypeSize());
        *data = (char*) malloc(*size);
        memcpy(*data, (char*)d->GetVoidPointer(0), *size);
}

void DNLImageExtractor::setLayer(int layer) {
    this->layer = layer;
}

PatientMetadata DNLImageExtractor::getPatientMetadata(DNLImage::Pointer image) {
    PatientMetadata patient;
    patient.name = image->patientName();
    return patient;
}
