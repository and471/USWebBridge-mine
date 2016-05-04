#ifndef DNLFRAMEEXCHANGE_H
#define DNLFRAMEEXCHANGE_H

#include <mutex>
#include <Modules/USStreamingCommon/DNLImage.h>


class Frame {

public:
    Frame(char* data, size_t size);
    ~Frame();

    static Frame* copy(Frame* frame);
    char* getData();
    size_t getSize();

private:
    char* data;
    size_t size;
};


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
