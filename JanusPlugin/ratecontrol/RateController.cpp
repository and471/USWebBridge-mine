#include "RateController.h"
#include <functional>

void RateController::setOnBitrateChangeCallback(std::function<void(int)> cb) {
    onBitrateChangeCallback = cb;
}

void RateController::bitrateChange(int bitrate) {
    if (onBitrateChangeCallback) onBitrateChangeCallback(bitrate);
}

