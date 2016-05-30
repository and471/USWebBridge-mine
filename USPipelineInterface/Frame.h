#ifndef FRAME_H
#define FRAME_H

#include "PatientMetadata.h"
#include "ImageMetadata.h"

class Frame {

public:
    Frame(char* data, size_t size, PatientMetadata patientMetadata, ImageMetadata imageMetadata);
    ~Frame();

    static Frame* copy(Frame* frame);
    char* getData();
    size_t getSize();
    PatientMetadata getPatientMetadata();
    ImageMetadata getImageMetadata();

private:
    char* data;
    size_t size;
    PatientMetadata patientMetadata;
    ImageMetadata imageMetadata;
};


#endif // FRAME_H
