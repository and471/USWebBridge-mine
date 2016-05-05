#ifndef IMAGESOURCE_H_
#define IMAGESOURCE_H_

#include <map>
#include <functional>
#include "Frame.h"

class FrameSource {

public:
    int addOnFrameCallback(std::function<void(Frame*)> cb);
    void removeOnFrameCallback(int callbackID);

    int addOnNSlicesChangedCallback(std::function<void(int)> cb);
    void removeOnNSlicesChangedCallback(int callbackID);

    virtual ~FrameSource();
    virtual void setSlice(int slice) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;

protected:
    void onFrame(Frame* frame);
    void onNSlicesChanged();
    int slice;
    int nSlices;

private:
    int callbackIDCounter = 0;
    std::map<int, std::function<void(Frame*)>> onFrameCallbacks;
    std::map<int, std::function<void(int)>> onNSlicesChangedCallbacks;

};

#endif /// IMAGESOURCE_H_
