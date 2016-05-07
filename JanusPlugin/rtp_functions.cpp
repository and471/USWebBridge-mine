/*!
 * From Janus Streaming Plugin by Lorenzo Miniero <lorenzo@meetecho.com>
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
#include <poll.h>
}

#include "plugin_hooks.h"

#include "RTPSource.h"
#include "rtp_functions.h"
#include "janus_ultrasound.h"


/* Helpers to create a listener filedescriptor */
int create_fd(int port, in_addr_t mcast, const char* listenername, const char* medianame, const char* mountpointname) {
	struct sockaddr_in address;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0) {
		JANUS_LOG(LOG_ERR, "[%s] Cannot create socket for %s...\n", mountpointname, medianame);
		return -1;
	}	
	if(port > 0) {
		if(IN_MULTICAST(ntohl(mcast))) {
#ifdef IP_MULTICAST_ALL			
			int mc_all = 0;
			if((setsockopt(fd, IPPROTO_IP, IP_MULTICAST_ALL, (void*) &mc_all, sizeof(mc_all))) < 0) {
				JANUS_LOG(LOG_ERR, "[%s] %s listener setsockopt IP_MULTICAST_ALL failed\n", mountpointname, listenername);
				close(fd);
				return -1;
			}			
#endif			
			struct ip_mreq mreq;
			memset(&mreq, 0, sizeof(mreq));
			mreq.imr_multiaddr.s_addr = mcast;
			if(setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq)) == -1) {
				JANUS_LOG(LOG_ERR, "[%s] %s listener IP_ADD_MEMBERSHIP failed\n", mountpointname, listenername);
				close(fd);
				return -1;
			}
			JANUS_LOG(LOG_ERR, "[%s] %s listener IP_ADD_MEMBERSHIP ok\n", mountpointname, listenername);
		}
	}

	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;
	if(bind(fd, (struct sockaddr *)(&address), sizeof(struct sockaddr)) < 0) {
		JANUS_LOG(LOG_ERR, "[%s] Bind failed for %s (port %d)...\n", mountpointname, medianame, port);
		close(fd);
		return -1;
	}
	return fd;
}


/* FIXME Test thread to relay RTP frames coming from gstreamer/ffmpeg/others */
void* relay_rtp_thread(void *data) {
	JANUS_LOG(LOG_VERB, "Starting ultrasound relay thread\n");
    RTPSource* mountpoint = (RTPSource*) data;
	if(!mountpoint) {
		JANUS_LOG(LOG_ERR, "Invalid mountpoint!\n");
		g_thread_unref(g_thread_self());
		return NULL;
	}

    int video_fd = mountpoint->video_fd;
	char *name = g_strdup(mountpoint->name ? mountpoint->name : "??");
	/* Needed to fix seq and ts */
    uint32_t v_last_ssrc = 0, v_last_ts = 0, v_base_ts = 0, v_base_ts_prev = 0;
    uint16_t v_last_seq = 0, v_base_seq = 0, v_base_seq_prev = 0;
	/* File descriptors */
	socklen_t addrlen;
	struct sockaddr_in remote;
	int resfd = 0, bytes = 0;
	struct pollfd fds[2];
	char buffer[1500];
	memset(buffer, 0, 1500);
	/* Loop */
	int num = 0;
    rtp_relay_packet packet;
    while(!g_atomic_int_get(&stopping)) {
		/* Prepare poll */
		num = 0;
		if(video_fd != -1) {
			fds[num].fd = video_fd;
			fds[num].events = POLLIN;
			fds[num].revents = 0;
			num++;
		}
		/* Wait for some data */
		resfd = poll(fds, num, 1000);
		if(resfd < 0) {
			JANUS_LOG(LOG_ERR, "[%s] Error polling... %d (%s)\n", mountpoint->name, errno, strerror(errno));
			mountpoint->enabled = FALSE;
			break;
		} else if(resfd == 0) {
			/* No data, keep going */
			continue;
		}
		int i = 0;
		for(i=0; i<num; i++) {
			if(fds[i].revents & (POLLERR | POLLHUP)) {
				/* Socket error? */
				JANUS_LOG(LOG_ERR, "[%s] Error polling: %s... %d (%s)\n", mountpoint->name,
					fds[i].revents & POLLERR ? "POLLERR" : "POLLHUP", errno, strerror(errno));
				mountpoint->enabled = FALSE;
				break;
			} else if(fds[i].revents & POLLIN) {
				/* Got an RTP packet */
                if(video_fd != -1 && fds[i].fd == video_fd) {
					/* Got something video (RTP) */
					if(mountpoint->active == FALSE)
						mountpoint->active = TRUE;
                    mountpoint->last_received_video = janus_get_monotonic_time();
					addrlen = sizeof(remote);
					bytes = recvfrom(video_fd, buffer, 1500, 0, (struct sockaddr*)&remote, &addrlen);
					//~ JANUS_LOG(LOG_VERB, "************************\nGot %d bytes on the video channel...\n", bytes);
					rtp_header *rtp = (rtp_header *)buffer;

					/* If paused, ignore this packet */
					if(!mountpoint->enabled)
						continue;
					//~ JANUS_LOG(LOG_VERB, " ... parsed RTP packet (ssrc=%u, pt=%u, seq=%u, ts=%u)...\n",
						//~ ntohl(rtp->ssrc), rtp->type, ntohs(rtp->seq_number), ntohl(rtp->timestamp));
					/* Relay on all sessions */
					packet.data = rtp;
                    packet.length = bytes;
					/* Do we have a new stream? */
					if(ntohl(packet.data->ssrc) != v_last_ssrc) {
						v_last_ssrc = ntohl(packet.data->ssrc);
						JANUS_LOG(LOG_INFO, "[%s] New video stream! (ssrc=%u)\n", name, v_last_ssrc);
						v_base_ts_prev = v_last_ts;
						v_base_ts = ntohl(packet.data->timestamp);
						v_base_seq_prev = v_last_seq;
						v_base_seq = ntohs(packet.data->seq_number);
					}
					v_last_ts = (ntohl(packet.data->timestamp)-v_base_ts)+v_base_ts_prev+4500;	/* FIXME We're assuming 15fps here... */
					packet.data->timestamp = htonl(v_last_ts);
					v_last_seq = (ntohs(packet.data->seq_number)-v_base_seq)+v_base_seq_prev+1;
					packet.data->seq_number = htons(v_last_seq);
					//~ JANUS_LOG(LOG_VERB, " ... updated RTP packet (ssrc=%u, pt=%u, seq=%u, ts=%u)...\n",
						//~ ntohl(rtp->ssrc), rtp->type, ntohs(rtp->seq_number), ntohl(rtp->timestamp));
					packet.data->type = mountpoint->codecs.video_pt;
					/* Backup the actual timestamp and sequence number set by the restreamer, in case switching is involved */
					packet.timestamp = ntohl(packet.data->timestamp);
					packet.seq_number = ntohs(packet.data->seq_number);
					/* Go! */
					janus_mutex_lock(&mountpoint->mutex);
                    g_list_foreach(mountpoint->listeners, relay_rtp_packet, &packet);
					janus_mutex_unlock(&mountpoint->mutex);
					continue;
				}
			}
		}
	}

	/* Notify users this mountpoint is done */
	janus_mutex_lock(&mountpoint->mutex);
	GList *viewer = g_list_first(mountpoint->listeners);
	/* Prepare JSON event */
	json_t *event = json_object();
	json_object_set_new(event, "ultrasound", json_string("event"));
	json_t *result = json_object();
	json_object_set_new(result, "status", json_string("stopped"));
	json_object_set_new(event, "result", result);
	char *event_text = json_dumps(event, JSON_INDENT(3) | JSON_PRESERVE_ORDER);
	json_decref(event);
	while(viewer) {
		janus_ultrasound_session *session = (janus_ultrasound_session *)viewer->data;
		if(session != NULL) {
			session->stopping = TRUE;
            session->started = FALSE;
			session->mountpoint = NULL;
			/* Tell the core to tear down the PeerConnection, hangup_media will do the rest */
			gateway->push_event(session->handle, &janus_ultrasound_plugin, NULL, event_text, NULL, NULL);
			gateway->close_pc(session->handle);
		}
		mountpoint->listeners = g_list_remove_all(mountpoint->listeners, session);
		viewer = g_list_first(mountpoint->listeners);
	}
	g_free(event_text);
	janus_mutex_unlock(&mountpoint->mutex);

    delete mountpoint;

	JANUS_LOG(LOG_VERB, "[%s] Leaving ultrasound relay thread\n", name);
	g_free(name);
	g_thread_unref(g_thread_self());
	return NULL;
}

void relay_rtp_packet(gpointer data, gpointer user_data) {
    rtp_relay_packet *packet = (rtp_relay_packet *)user_data;
    if(!packet || !packet->data || packet->length < 1) {
        JANUS_LOG(LOG_ERR, "Invalid packet...\n");
        return;
    }
    janus_ultrasound_session *session = (janus_ultrasound_session *)data;
    if(!session || !session->handle) {
        //~ JANUS_LOG(LOG_ERR, "Invalid session...\n");
        return;
    }

    /* Make sure there hasn't been a publisher switch by checking the SSRC */

    if(ntohl(packet->data->ssrc) != session->context.v_last_ssrc) {
        session->context.v_last_ssrc = ntohl(packet->data->ssrc);
        session->context.v_base_ts_prev = session->context.v_last_ts;
        session->context.v_base_ts = packet->timestamp;
        session->context.v_base_seq_prev = session->context.v_last_seq;
        session->context.v_base_seq = packet->seq_number;
    }
    /* Compute a coherent timestamp and sequence number */
    session->context.v_last_ts = (packet->timestamp-session->context.v_base_ts)
        + session->context.v_base_ts_prev+4500;	/* FIXME When switching, we assume 15fps */
    session->context.v_last_seq = (packet->seq_number-session->context.v_base_seq)+session->context.v_base_seq_prev+1;
    /* Update the timestamp and sequence number in the RTP packet, and send it */
    packet->data->timestamp = htonl(session->context.v_last_ts);
    packet->data->seq_number = htons(session->context.v_last_seq);
    if (session->gateway != NULL)
        session->gateway->relay_rtp(session->handle, true, (char *)packet->data, packet->length);
    /* Restore the timestamp and sequence number to what the publisher set them to */
    packet->data->timestamp = htonl(packet->timestamp);
    packet->data->seq_number = htons(packet->seq_number);


    return;
}
