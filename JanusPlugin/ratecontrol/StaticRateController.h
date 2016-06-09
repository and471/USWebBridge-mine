#ifndef STATICRATECONTROLLER_H
#define STATICRATECONTROLLER_H


class StaticRateController : public RateController
{
public:
    void onRTCPReceiverReport(rtcp_rr* rr, struct timeval arrival);

private:
};

#endif
