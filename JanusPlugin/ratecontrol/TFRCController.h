#ifndef TFRCCONTROLLER_H
#define TFRCCONTROLLER_H

#define BITRATE_MIN (200 * 1000)
#define BITRATE_MAX (3   * 1000 * 1000)

class PacketLossTracker {

public:
    void add(double loss);
    double get();
    void reset();
private:
    int loss = 0;
    int n = 0;

};

class PeakTracker {

public:
    void add(int bitrate);
    int get();
    bool close(int bitrate);
private:
    int peak = BITRATE_MAX;
    int lastBitrate = BITRATE_MIN;
    int currentBitrate = BITRATE_MIN;

};

class TFRCController : public RateController
{
public:
    void onRTCPReceiverReport(rtcp_rr* rr, struct timeval arrival);
    void setOnBitrateChangeCallback(std::function<void(int)> cb);

private:
    const double BITRATE_CHANGE_MAX = 0.08; //40

    const int CHANGE_INTERVAL = 1500;
    const int CHANGE_INTERVAL_PEAK = 3000;

    int calculateInitialBitrate(int MSS, int R);
    int calculateBitrate(double R_sample);
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
    long int lastChanged = -1;
    long int start = -1;

    PeakTracker* peak = new PeakTracker();

    bool slowStart = true;
};


#endif
