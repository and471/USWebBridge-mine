#include "DNLImageExtractor.h"
#include <vtkJPEGWriter.h>
#include <vtkUnsignedCharArray.h>

void DNLImageExtractor::get_jpeg(DNLImage::Pointer image, char** data, size_t* size) {
    vtkSmartPointer<vtkJPEGWriter> writer = vtkSmartPointer<vtkJPEGWriter>::New();
    writer->SetWriteToMemory(true);
    writer->SetInputData(image->GetVTKImage(0));
    writer->Write();

    vtkUnsignedCharArray* d = writer->GetResult();

    *data = (char*) d->GetVoidPointer(1);
    *size = d->GetActualMemorySize() * 1024;
}

