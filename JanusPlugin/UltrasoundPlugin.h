#ifndef ULTRASOUNDPLUGIN_H
#define ULTRASOUNDPLUGIN_H

extern "C" {
#include <janus/debug.h>
#include <janus/apierror.h>
#include <janus/config.h>
#include <janus/mutex.h>
#include <janus/rtp.h>
#include <janus/rtcp.h>
#include <janus/record.h>
#include <janus/utils.h>
#include <janus/plugin.h>
#include "plugin_hooks.h"
}

#include <USPipelineInterface/interface.h>
#include <USPipelineInterface/PatientMetadata.h>
#include <boost/format.hpp>
#include <functional>

class UltrasoundPlugin
{
public:
    UltrasoundPlugin(USPipelineInterface* interface, janus_callbacks* gateway, janus_plugin_session* handle);

    void onNewPatientMetadata(PatientMetadata patient);
    void sendData(char* data);
    void sendData(boost::format data);
    void start();

    static std::string patientMetadataToJSON(PatientMetadata patient);

private:
    USPipelineInterface* interface;
    janus_callbacks* gateway;
    janus_plugin_session* handle;
    bool started = false;

    static const char* NEW_PATIENT_METADATA = \
    "{                                          \
        name: 'NEW_PATIENT_METADATA',           \
        data: %s                                \
    }";

    static const char* PATIENT_METADATA = \
    "{                                          \                                \
        name: '%s'                              \
        }                                       \
    }";

};

#endif // ULTRASOUNDPLUGIN_H
