#ifndef ULTRASOUNDPLUGIN_H
#define ULTRASOUNDPLUGIN_H

#include <USPipelineInterface/interface.h>

class UltrasoundPlugin
{
public:
    UltrasoundPlugin(USPipelineInterface* interface);

private:
    USPipelineInterface* interface;
};

#endif // ULTRASOUNDPLUGIN_H
