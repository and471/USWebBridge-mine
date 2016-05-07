/*!
 * From Janus Streaming Plugin by Lorenzo Miniero <lorenzo@meetecho.com>
 * see https://janus.conf.meetecho.com/
*/

#ifndef RTP_FUNCTIONS_H
#define RTP_FUNCTIONS_H

extern "C" {
#include <glib.h>
#include <janus/rtp.h>
}

typedef struct rtp_relay_packet {
    rtp_header *data;
    gint length;
    uint32_t timestamp;
    uint16_t seq_number;
} rtp_relay_packet;

/* Helpers to create a listener filedescriptor */
int create_fd(int port, in_addr_t mcast, const char* listenername,
                     const char* medianame, const char* mountpointname);

/* Thread to relay RTP frames */
void* relay_rtp_thread(void *data);

void relay_rtp_packet(gpointer data, gpointer user_data);


#endif
