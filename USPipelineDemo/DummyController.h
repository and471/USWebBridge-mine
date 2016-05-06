#ifndef DUMMY_CONTROLLER_H
#define DUMMY_CONTROLLER_H

#include <functional>

#include <USPipelineInterface/PatientMetadata.h>
#include <USPipelineInterface/FrameSource.h>
#include <USPipelineInterface/UltrasoundImagePipeline.h>
#include <USPipelineInterface/UltrasoundController.h>

class DummyController : public UltrasoundController
{
public:
    DummyController();
    ~DummyController();

    // Virtual methods
    void start();
    void stop();
    void onNewPatientMetadata(PatientMetadata patient);
    void onNSlicesChanged(int nSlices);
    void onSetSlice(int slice);
    void setOnSetSliceCallback(std::function<void(int)> cb);
    void setPipeline(UltrasoundImagePipeline* pipeline);

private:
    bool started = false;

};

#endif // DUMMY_CONTROLLER_H
