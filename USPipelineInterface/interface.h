#ifndef USPIPELINEINTERFACE_H
#define USPIPELINEINTERFACE_H

#include "interface.h"
#include <map>

static int SIGNAL_PIPELINE_MESSAGE = 0;

class UltrasoundImagePipeline; // forward declare to avoid cyclic dependency

class USPipelineInterface {

public:
    USPipelineInterface();
    ~USPipelineInterface();

    void connect(int signal, void (*callback)(void*));
    void fire(int signal, void* data);

private:
    UltrasoundImagePipeline* pipeline;
    std::map<int, void (*)(void*)>* callbacks;

    void (*on_pipeline_message)(char*);

};
#endif // USPIPELINEINTERFACE_H
