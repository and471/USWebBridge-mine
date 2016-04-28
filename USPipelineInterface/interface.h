#ifndef USPIPELINEINTERFACE_H
#define USPIPELINEINTERFACE_H

#include <functional>

#include "interface.h"
#include "PatientMetadata.h"

class UltrasoundImagePipeline; // forward declare to avoid cyclic dependency

class USPipelineInterface {

public:
    USPipelineInterface();
    ~USPipelineInterface();

    void setOnNewPatientMetadataCallback(std::function<void(PatientMetadata)> cb);
    void OnNewPatientMetadata(PatientMetadata patient);
    void start();

private:
    UltrasoundImagePipeline* pipeline;


    std::function<void(PatientMetadata)> onNewPatientMetadataCallback;

    void (*on_pipeline_message)(char*);

};
#endif // USPIPELINEINTERFACE_H
