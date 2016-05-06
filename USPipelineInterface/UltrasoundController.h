#ifndef ULTRASOUNDPLUGIN_H
#define ULTRASOUNDPLUGIN_H

#include "UltrasoundImagePipeline.h"
#include "PatientMetadata.h"
#include <functional>

class UltrasoundImagePipeline; // forward declaration
class USPipelineInterface; // forward declaration

class UltrasoundController
{
public:
    virtual void start() =0;
    virtual void stop()=0;

    virtual void onNewPatientMetadata(PatientMetadata patient) =0;
    virtual void onNSlicesChanged(int nSlices) =0;
    virtual void onSetSlice(int slice) =0;
    virtual void setOnSetSliceCallback(std::function<void(int)> cb) =0; 
    virtual void setPipeline(UltrasoundImagePipeline* pipeline) =0;

protected:
    UltrasoundImagePipeline* pipeline;
    bool started = false;
    std::function<void(int)> onSetSliceCallback;

};

#endif // ULTRASOUNDPLUGIN_H
