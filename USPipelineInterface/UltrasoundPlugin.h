#ifndef ULTRASOUNDPLUGIN_H
#define ULTRASOUNDPLUGIN_H

#include "interface.h"
#include <functional>

class USPipelineInterface; // forward declaration

class UltrasoundPlugin
{
public:
    virtual void start() =0;

    virtual void onNewPatientMetadata(PatientMetadata patient) =0;
    virtual void onNSlicesChanged(int nSlices) =0;
    virtual void onSetSlice(int slice) =0;
    virtual void setOnSetSliceCallback(std::function<void(int)> cb) =0;

protected:
    USPipelineInterface* interface;
    bool started = false;
    std::function<void(int)> onSetSliceCallback;

};

#endif // ULTRASOUNDPLUGIN_H
