#ifndef RTP_SOURCE_H
#define RTP_SOURCE_H


#include <janus/debug.h>
#include <janus/apierror.h>
#include <janus/config.h>
#include <janus/mutex.h>
#include <janus/rtp.h>
#include <janus/rtcp.h>
#include <janus/record.h>
#include <janus/utils.h>
#include <janus/plugin.h>

#include "janus_ultrasound.h"

class RTPSource {

public:
    RTPSource(int id, char* name, int video_port, int video_fd, int video_codec,
              char* video_rtpmap);
    ~RTPSource();

    char* createSDP();

	int id;
	char* name = nullptr;
	char* secret = nullptr;

	janus_ultrasound_codecs codecs;
    GList* listeners;
    janus_mutex mutex;

    bool enabled;
    bool active;

    int video_port;
    in_addr_t video_mcast;
    int video_fd;
    int last_received_video;
};

#endif
