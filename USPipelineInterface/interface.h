#ifndef USPIPELINEINTERFACE_H
#define USPIPELINEINTERFACE_H

#include <functional>

#include "interface.h"
#include "PatientMetadata.h"
#include "UltrasoundPlugin.h"
#include "UltrasoundImagePipeline.h"

class UltrasoundPlugin;
class UltrasoundImagePipeline; // forward declare to avoid cyclic dependency

class USPipelineInterface {

public:
    USPipelineInterface();
    ~USPipelineInterface();

    void setPlugin(UltrasoundPlugin* plugin);

    void setOnNewPatientMetadataCallback(std::function<void(PatientMetadata)> cb);
    void onNewPatientMetadata(PatientMetadata patient);

    void setOnNSlicesChangedCallback(std::function<void(int)> cb);
    void onNSlicesChanged(int nSlices);

    void setOnSetSliceCallback(std::function<void(int)> cb);
    void onSetSlice(int slice);

    void start();

private:
    UltrasoundImagePipeline* pipeline;
    UltrasoundPlugin* plugin;

    std::function<void(PatientMetadata)> onNewPatientMetadataCallback;
    std::function<void(int)> onNSlicesChangedCallback;
    std::function<void(int)> onSetSliceCallback;

};
#endif // USPIPELINEINTERFACE_H
