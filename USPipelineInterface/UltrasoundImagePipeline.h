#ifndef ULTRASOUNDIMAGEPIPELINE_H
#define ULTRASOUNDIMAGEPIPELINE_H

#include "UltrasoundController.h"
#include "PatientMetadata.h"
#include "FrameSource.h"
#include <functional>

class UltrasoundController; // forward declaration

class UltrasoundImagePipeline
{
public:
    virtual ~UltrasoundImagePipeline() {};

    virtual void start()=0;
    virtual void stop()=0;

    virtual void onSetSlice(int slice)=0;
    virtual void setOnNewPatientMetadataCallback(std::function<void(PatientMetadata)> cb) =0;
    virtual void setOnNewImageMetadataCallback(std::function<void(ImageMetadata)> cb) =0;

    virtual void setFrameSource(FrameSource* frame_source) =0;

    virtual int getPort() =0;

protected:
    UltrasoundController* controller;
    FrameSource* frame_source;
    int fps = 20;

    std::function<void(PatientMetadata)> onNewPatientMetadataCallback;
    std::function<void(ImageMetadata)> onNewImageMetadataCallback;

    virtual void onFrame(Frame* frame) =0;
    virtual void onNSlicesChanged(int nSlices) =0;
};

#endif // ULTRASOUNDIMAGEPIPELINE_H
