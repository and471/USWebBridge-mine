#ifndef ULTRASOUNDIMAGEPIPELINE_H
#define ULTRASOUNDIMAGEPIPELINE_H

#include "UltrasoundPlugin.h"

class UltrasoundImagePipeline
{
public:
    virtual void start()=0;
    virtual void stop()=0;

    virtual void onNSlicesChanged(int nSlices)=0;
    virtual void onSetSlice(int slice)=0;

protected:
    UltrasoundPlugin* plugin;
    int fps = 20;
};

#endif // ULTRASOUNDIMAGEPIPELINE_H
