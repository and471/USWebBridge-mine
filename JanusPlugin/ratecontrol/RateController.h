#ifndef RATECONTROLLER_H
#define RATECONTROLLER_H

#include <functional>
#include <glib.h>
#include <janus/rtcp.h>

class RateController
{
public:
    virtual void onRTCPReceiverReport(rtcp_rr* rr, struct timeval arrival) =0;

    virtual void setOnBitrateChangeCallback(std::function<void(int)> cb);

protected:
    void bitrateChange(int bitrate);
    std::function<void(int)> onBitrateChangeCallback;

};

#endif // RATECONTROLLER_H
