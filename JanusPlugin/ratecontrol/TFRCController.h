#ifndef TFRCCONTROLLER_H
#define TFRCCONTROLLER_H


class PacketLossTracker {

public:
    void add(double loss);
    double get();
    void reset();
private:
    int loss = 0;
    int n = 0;

};

class TFRCController : public RateController
{
public:
    void onRTCPReceiverReport(rtcp_rr* rr, struct timeval arrival);
    void setOnBitrateChangeCallback(std::function<void(int)> cb);

private:
    const int BITRATE_MIN = 200 * 1000;
    const int BITRATE_MAX = 2   * 1000 * 1000;
    const double BITRATE_CHANGE_MAX = 15*1000; //15
    const double BITRATE_CHANGE_MAX_SLOWSTART = 0.08; //40

    const int CHANGE_INTERVAL_DECREASE = 3000;
    const int CHANGE_INTERVAL_INCREASE = 4500;
    const int CHANGE_INTERVAL_SLOWSTART = 1000;

    int calculateInitialBitrate(int MSS, int R);
    int calculateBitrate(double R_sample, bool* newSlowStart);
    void updateRTT(double R_new);
    uint32_t getNTPTime32(struct timeval tv);
    long int getMilliseconds();

    // Round trip time estimate (secs)
    double R = -1;
    // Exponential averaging weight
    double q = 0.9;

    // Square root of round trip time estimate (secs)
    // https://tools.ietf.org/html/rfc5348#section-4.5
    double R_sqmean = -1;
    // Exponential averaging weight
    double q2 = 0.9;

    // Segement size (= maximum segment size)
    int s = 1024;

    // Fraction of packet's lost
    PacketLossTracker* packetLoss = new PacketLossTracker();

    int currentBitrate = BITRATE_MIN;
    int lastCollapse = BITRATE_MAX;
    long int lastChanged = -1;
    long int start = -1;
    int lastChange = 0;

    bool slowStart = true;
};


#endif
