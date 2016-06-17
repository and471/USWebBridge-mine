#include "StreamAdaptor.h"

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)

void StreamAdaptor::setProfile(StreamProfile profile) {
    this->profile = profile;
}


int StreamAdaptor::getFPS(int bitrate) {
    int FPS_min, FPS_max, FPS_baseline,
        B_min, B_max, B_baseline;

    if (profile == NONE) {
        return -1;
    } else if (profile == _2D) {
        FPS_min = 4;
        FPS_max = 16;
        FPS_baseline = 8;
        B_min = 200 * 1000;
        B_max = 3 * 1000 * 1000;
        B_baseline = 600 * 1000;
    } else {
        // Don't decrease FPS for 3D scans as it is low enough already
        return 3;
    }

    int FPS;
    if (B_min <= bitrate <= B_max) {
        FPS = FPS_min + (FPS_baseline - FPS_min) * (double)(bitrate - B_min)/(B_baseline - B_min);
    } else {
        FPS = FPS_baseline + (FPS_max - FPS_baseline) * (double)(bitrate - B_baseline)/(B_max - B_baseline);
    }

    return min(max(FPS_min, FPS), FPS_max);
}
