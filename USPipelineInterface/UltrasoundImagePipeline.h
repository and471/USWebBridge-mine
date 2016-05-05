#ifndef ULTRASOUNDIMAGEPIPELINE_H
#define ULTRASOUNDIMAGEPIPELINE_H

#include "UltrasoundPlugin.h"
#include "PatientMetadata.h"
#include "FrameSource.h"
#include <functional>

class UltrasoundController; // forward declaration

class UltrasoundImagePipeline
{
public:
    virtual void start()=0;
    virtual void stop()=0;

    virtual void onSetSlice(int slice)=0;

    virtual void setOnNewPatientMetadataCallback(std::function<void(PatientMetadata)> cb) =0;
    virtual void onNewPatientMetadata(PatientMetadata patient) =0;

    virtual void setFrameSource(FrameSource* frame_source) =0;

    virtual int getPort() =0;

protected:
    UltrasoundController* controller;
    FrameSource* frame_source;
    int fps = 20;

    std::function<void(PatientMetadata)> onNewPatientMetadataCallback;
};

#endif // ULTRASOUNDIMAGEPIPELINE_H
