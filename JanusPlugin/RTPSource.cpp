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
#include <boost/format.hpp>
#include <string>

RTPSource::RTPSource(int id, char* name, int video_port, int video_fd, int video_codec,
                     char* video_rtpmap)
{
    this->id = id;
    this->name = name;
    this->enabled = TRUE;
    this->active = FALSE;
    this->video_mcast = INADDR_ANY;
    this->video_port = video_port;
    this->video_fd = video_fd;
    this->last_received_video = janus_get_monotonic_time();
    this->codecs.video_pt = video_codec;
    this->codecs.video_rtpmap = video_rtpmap;
    this->listeners = NULL;
    janus_mutex_init(&this->mutex);
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
}


char* RTPSource::createSDP() {
    gint64 sessid = janus_get_real_time();
    gint64 version = sessid;
    std::string general = str(boost::format("%s\r\n%s\r\n%s\r\n%s\r\n") %
        "v=0" %
        str(boost::format("o=- %llu %llu IN IP4 127.0.0.1") % sessid % version) %
        "s=Streaming Test" %
        "t=0 0"
    );

    std::string video = "";
    if(codecs.video_pt >= 0) {
        video = str(boost::format("%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n") %
            str(boost::format("m=video 1 RTP/SAVPF %d") % codecs.video_pt) %
            "c=IN IP4 1.1.1.1" %
            str(boost::format("a=rtpmap:%d %s") % codecs.video_pt % codecs.video_rtpmap) %
            str(boost::format("a=rtcp-fb:%d nack") % codecs.video_pt) %
            str(boost::format("a=rtcp-fb:%d goog-remb") % codecs.video_pt) %
            "a=sendonly"
        );
    }

    std::string data = str(boost::format("%s\r\n%s\r\n%s\r\n") %
       "m=application 1 DTLS/SCTP 5000" %
       "c=IN IP4 1.1.1.1" %
       "a=sctpmap:5000 webrtc-datachannel 16"
    );

    std::string sdp = general + video + data;

    return strdup(sdp.c_str());
}
