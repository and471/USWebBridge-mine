#include "plugin_hooks.h"

int janus_ultrasound_get_api_compatibility(void) {
	/* Important! This is what your plugin MUST always return: don't lie here or bad things will happen */
	return JANUS_PLUGIN_API_VERSION;
}

int janus_ultrasound_get_version(void) {
	return JANUS_ULTRASOUND_VERSION;
}

const char *janus_ultrasound_get_version_string(void) {
	return JANUS_ULTRASOUND_VERSION_STRING;
}

const char *janus_ultrasound_get_description(void) {
	return JANUS_ULTRASOUND_DESCRIPTION;
}

const char *janus_ultrasound_get_name(void) {
	return JANUS_ULTRASOUND_NAME;
}

const char *janus_ultrasound_get_author(void) {
	return JANUS_ULTRASOUND_AUTHOR;
}

const char *janus_ultrasound_get_package(void) {
	return JANUS_ULTRASOUND_PACKAGE;
}
