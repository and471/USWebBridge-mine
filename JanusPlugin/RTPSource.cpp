/*!
 * Heavily based on Janus Streaming Plugin by Lorenzo Miniero <lorenzo@meetecho.com>
 * see https://janus.conf.meetecho.com/
*/

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
}

#include "plugin_hooks.h"
#include "janus_ultrasound.h"
#include "RTPSource.h"

RTPSource::RTPSource() {

}

RTPSource::~RTPSource() {
    free(name);
	free(secret);

    janus_mutex_lock(&mutex);
	g_list_free(listeners);
    janus_mutex_unlock(&mutex);

    if(video_fd > 0) {
        close(video_fd);
    }

	free(codecs.video_rtpmap);
	free(codecs.video_fmtp);
}
