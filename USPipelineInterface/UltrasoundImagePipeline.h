#ifndef ULTRASOUNDIMAGEPIPELINE_H
#define ULTRASOUNDIMAGEPIPELINE_H

#include "UltrasoundPlugin.h"
#include "PatientMetadata.h"
#include <functional>

class UltrasoundPlugin; // forward declaration

class UltrasoundImagePipeline
{
public:
    virtual void start()=0;
    virtual void stop()=0;

    virtual void onSetSlice(int slice)=0;

    virtual void setOnNewPatientMetadataCallback(std::function<void(PatientMetadata)> cb) =0;
    virtual void onNewPatientMetadata(PatientMetadata patient) =0;

protected:
    UltrasoundPlugin* plugin;
    int fps = 20;

    std::function<void(PatientMetadata)> onNewPatientMetadataCallback;
};

#endif // ULTRASOUNDIMAGEPIPELINE_H
