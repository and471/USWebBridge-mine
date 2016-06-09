#include "RateController.h"
#include "TFRCController.h"
#include <math.h>
#include <sys/time.h>

int min(int a, int b) { return (a < b) ? a : b; }
int max(int a, int b) { return (a > b) ? a : b; }

void TFRCController::onRTCPReceiverReport(rtcp_rr* rr, struct timeval arrival) {
    if (start == -1) start = getMilliseconds();

    // Last sender report timestamp (from sender)
    uint32_t lsr = ntohl(rr->rb[0].lsr);
    // No sender report has been sent yet
    if (lsr == 0) return;

    // Delay between last sender report and sending this receiver report
    uint32_t dlsr = ntohl(rr->rb[0].delay);

    // Calculate estimate of RTT
    // From https://tools.ietf.org/html/rfc3550#section-6.4.1 page 40
    uint32_t A = getNTPTime32(arrival);
    double rtt = (A - dlsr - lsr) / 65536.;

    updateRTT(rtt);

    uint32_t fractionLostFP = ntohl(rr->rb[0].flcnpl) >> 24;
    p = fractionLostFP / 256.;

    // If estimated RTT is not yet available, don't calculate bitrate
    if (R == -1) return;

    bool newSlowStart = slowStart;
    int bitrate = calculateBitrate(rtt, &newSlowStart);

    int interval;

    if (slowStart) {
        interval = CHANGE_INTERVAL_SLOWSTART;
    } else {
        if (lastChange > 0) interval = CHANGE_INTERVAL_DECREASE;
        else                interval = CHANGE_INTERVAL_INCREASE;
    }

    if (getMilliseconds() - lastChanged  >= interval || lastChanged == -1) {
        bitrateChange(bitrate);
        lastChange = currentBitrate - bitrate;
        lastChanged = getMilliseconds();

        currentBitrate = bitrate;
    }

    slowStart = newSlowStart;
}

int TFRCController::calculateInitialBitrate(int MSS, int R) {
    return min(4 * MSS, max(2*MSS, 4380)) / R;
}

int TFRCController::calculateBitrate(double R_sample, bool* newSlowStart) {
    // Adapted from https://tools.ietf.org/html/rfc5348#section-3.1
    // s = segment size, R = round trip time, p = fraction of packets lost

    // Assume a minute packet loss
    p = (p == 0) ? 0.001 : p;

    int bitrate = 8. * s / (
        R * (sqrt(2.*p/3.) + 12.*sqrt(3.*p/8.)*p*(1.+32.*p*p))
    );

    bitrate = max(min(bitrate, BITRATE_MAX), BITRATE_MIN);

    if (!slowStart) {
        bitrate = bitrate * R_sqmean / sqrt(R_sample);
        fflush(stdout);
    }

    int max_change = BITRATE_CHANGE_MAX;
    if (slowStart) max_change = currentBitrate * BITRATE_CHANGE_MAX_SLOWSTART;

    // Close to last collapse, half max change
    // But if collapse was premature, allow to go past it quickly
    if (currentBitrate > lastCollapse * 0.75 && currentBitrate < lastCollapse) {
        max_change /= 4;
        fflush(stdout);
    }
    bitrate = min(bitrate, currentBitrate + max_change);

    if (bitrate <= currentBitrate * 0.75 || bitrate == BITRATE_MIN) {
        printf("Collapse at %d, as bitrate is %d\n", currentBitrate, bitrate);
        fflush(stdout);

        *newSlowStart = false;
        lastCollapse = currentBitrate;
    }

    bitrate = max(bitrate, currentBitrate/1.5);

    bitrate = max(min(bitrate, BITRATE_MAX), BITRATE_MIN);

    return bitrate;
}

void TFRCController::updateRTT(double R_sample) {
    if (R_sample > 60) {
        // Sometimes get erroneous measurements
        return;
    }

    if (R == -1) {
        R = R_sample;
        R_sqmean = sqrt(R_sample);
    } else {
        // Update estimate for RTT via exponential averaging
        R = q*R + (1-q)*R_sample;
        R_sqmean = q2*R_sqmean + (1-q2)*sqrt(R_sample);
    }
}

uint32_t TFRCController::getNTPTime32(struct timeval tv) {
    // Returns the current time in 32-bit NTP format
    // seconds since 0AM January 1 1900, in fixed point format (16bits for integer, 16bits for fractional)
    // truncated by 8 bits from each side

    /* From janus/ice.c */
    uint32_t s = tv.tv_sec + 2208988800u;
    uint32_t u = tv.tv_usec;
    uint32_t f = (u << 12) + (u << 8) - ((u * 3650) >> 6);
    /********************/

    return (s << 16) | (f >> 16);
}

void TFRCController::setOnBitrateChangeCallback(std::function<void(int)> cb) {
    RateController::setOnBitrateChangeCallback(cb);
    bitrateChange(BITRATE_MIN);
}

long int TFRCController::getMilliseconds() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec*1e3 + time.tv_usec*1e-3;
}
