#include "UltrasoundPlugin.h"
#include <USPipelineInterface/interface.h>
#include <USPipelineInterface/PatientMetadata.h>
#include <cstdio>
#include <functional>
#include <boost/format.hpp>

using namespace std::placeholders;

UltrasoundPlugin::UltrasoundPlugin(USPipelineInterface* interface, janus_callbacks* gateway, janus_plugin_session* handle) {
    this->interface = interface;
    this->gateway = gateway;
    this->handle = handle;

    //interface->setOnNewPatientMetadataCallback(std::bind(&UltrasoundPlugin::onNewPatientMetadata, this, _1));
}

/*void UltrasoundPlugin::onNewPatientMetadata(PatientMetadata patient) {
    boost::format format = boost::format(NEW_PATIENT_METADATA) % patientMetadataToJSON(patient);
    this->sendData(format);
}*/

void UltrasoundPlugin::sendData(boost::format data) {
    sendData(data.str().c_str());
}


void UltrasoundPlugin::sendData(char* data) {
    gateway->relay_data(handle, data, strlen(data));
}

void UltrasoundPlugin::start() {
    if (!started) {
        interface->start();
    }
}

std::string UltrasoundPlugin::patientMetadataToJSON(PatientMetadata patient) {
    //boost::format format = boost::format(UltrasoundPlugin::PATIENT_METADATA) % patient.name;
    return "";
}
