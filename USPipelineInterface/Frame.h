#ifndef FRAME_H
#define FRAME_H

#include "PatientMetadata.h"

class Frame {

public:
    Frame(char* data, size_t size, PatientMetadata patientMetadata);
    ~Frame();

    static Frame* copy(Frame* frame);
    char* getData();
    size_t getSize();
    PatientMetadata getPatientMetadata();

private:
    char* data;
    size_t size;
    PatientMetadata patientMetadata;
};


#endif // FRAME_H
