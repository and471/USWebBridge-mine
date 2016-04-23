#include "DNLImageExtractor.h"
#include <vtkJPEGWriter.h>
#include <vtkUnsignedCharArray.h>
#include <cstdio>

void DNLImageExtractor::get_jpeg(DNLImage::Pointer image, char** data, size_t* size) {
    vtkSmartPointer<vtkJPEGWriter> writer = vtkSmartPointer<vtkJPEGWriter>::New();
    //writer->SetWriteToMemory(true);

    writer->SetFileName("/tmp/frame.jpg");
    writer->SetInputData(image->GetVTKImage(0));
    writer->Write();

    //TODO: use SetWriteToMemory
    /*vtkUnsignedCharArray* d = writer->GetResult();
    *data = (char*) d->GetVoidPointer(1);
    *size = d->GetActualMemorySize() * 1024;*/

    FILE *f = fopen("/tmp/frame.jpg", "rb");
    fseek(f, 0, SEEK_END);
    *size = (size_t) ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);
    *data = (char*) malloc(*size);
    fread(*data, *size, sizeof(char), f);
    fclose(f);
}

