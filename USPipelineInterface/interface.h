#ifndef USPIPELINEINTERFACE_H
#define USPIPELINEINTERFACE_H

#include "UltrasoundPlugin.h"
#include "UltrasoundImagePipeline.h"

class UltrasoundPlugin;
class UltrasoundImagePipeline; // forward declare to avoid cyclic dependency

UltrasoundImagePipeline* initGstUltrasoundImagePipelineJanusPlugin(UltrasoundPlugin* plugin);

#endif // USPIPELINEINTERFACE_H
