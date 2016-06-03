#include <map>
#include <functional>

#include "Frame.h"
#include "FrameSource.h"

FrameSource::~FrameSource() {}

int FrameSource::addOnFrameCallback(std::function<void(Frame*)> cb) {
    onFrameCallbacks[callbackIDCounter] = cb;
    return callbackIDCounter++;
}

void FrameSource::removeOnFrameCallback(int callbackID) {
    onFrameCallbacks.erase(callbackID);
}

void FrameSource::onFrame(Frame* frame) {
    for (auto entry : onFrameCallbacks) {
        entry.second(frame);
    }
    delete frame;
}

int FrameSource::addOnNSlicesChangedCallback(std::function<void(int)> cb) {
    onNSlicesChangedCallbacks[callbackIDCounter] = cb;
    return callbackIDCounter++;
}

void FrameSource::removeOnNSlicesChangedCallback(int callbackID) {
    onNSlicesChangedCallbacks.erase(callbackID);
}

void FrameSource::onNSlicesChanged() {
    for (auto entry : onNSlicesChangedCallbacks) {
        entry.second(this->getNSlices());
    }
}
