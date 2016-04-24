#include "DNLImageExtractor.h"
#include <vtkPNGWriter.h>
#include <vtkUnsignedCharArray.h>
#include <cstdio>

void DNLImageExtractor::get_png(DNLImage::Pointer image, char** data, size_t* size) {
    vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
    writer->SetWriteToMemory(true);
    writer->SetInputData(image->GetVTKImage(0));
    writer->Write();

    // Move JPEG wrote in memory to new malloced location
    vtkSmartPointer<vtkUnsignedCharArray> d = writer->GetResult();
    *size = (size_t)(d->GetSize()*d->GetDataTypeSize());
    *data = (char*) malloc(*size);
    memcpy(*data, (char*)d->GetVoidPointer(0), *size);
}

