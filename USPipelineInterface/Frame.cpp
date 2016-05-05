#include "Frame.h"
#include "PatientMetadata.h"
#include <cstring>

Frame::Frame(char* data, size_t size, PatientMetadata patientMetadata) {
    this->size = size;
    this->data = (char*) malloc(size);
    memcpy(this->data, data, size);
}

Frame::~Frame() {
    free(this->data);
}

Frame* Frame::copy(Frame* frame) {
    Frame* new_frame = new Frame(frame->data, frame->size, frame->patientMetadata);
    return new_frame;
}

char* Frame::getData() {
    return data;
}

size_t Frame::getSize() {
    return size;
}

PatientMetadata Frame::getPatientMetadata() {
    return patientMetadata;
}
