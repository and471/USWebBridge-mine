#ifndef USPIPELINEINTERFACE_H
#define USPIPELINEINTERFACE_H

#include "UltrasoundPlugin.h"
#include "FrameSource.h"
#include "UltrasoundImagePipeline.h"

class UltrasoundController;
class UltrasoundImagePipeline; // forward declare to avoid cyclic dependency

FrameSource* getFrameSource();
UltrasoundImagePipeline* createPipeline(UltrasoundController* controller);

#endif // USPIPELINEINTERFACE_H
