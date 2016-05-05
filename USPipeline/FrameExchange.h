#ifndef DNLFRAMEEXCHANGE_H
#define DNLFRAMEEXCHANGE_H

#include <USPipelineInterface/Frame.h>
#include <mutex>

class FrameExchange
{
public:
    void add_frame(Frame* frame);
    Frame* get_frame();
    ~FrameExchange();

private:
    Frame* frame = nullptr;
    std::mutex mutex;
};

#endif // DNLFRAMEEXCHANGE_H
