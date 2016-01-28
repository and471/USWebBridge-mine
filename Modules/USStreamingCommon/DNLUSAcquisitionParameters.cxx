#include "DNLUSAcquisitionParameters.h"




bool DNLUSAcquisitionParameters::hasDepthChanged(DNLImage::Pointer next){

    return next->depthOfScanField() != this->depthOfScanField();

}

bool DNLUSAcquisitionParameters::hasAFrameDropped(DNLImage::Pointer next, int layer){

    uint64_t t1 = next->dnlLayerTimeTag()[layer];
    uint64_t t0 = this->dnlLayerTimeTag()[layer];
    uint64_t At = t1-t0;
    const double US2S = 1E-06; /// micro seconds to seconds

    double At_s = At*US2S;
    double th = 0.8; // a percent (1==100%) of time increase between two frames

    double measured_frame_rate = 1/At_s;
    double expected_frame_rate = this->acquisitionFrameRate()[layer];

    //std::cout << "DNLUSAcquisitionParameters::hasAFrameDropped - framerate (expected / measured)\t"<< expected_frame_rate <<"\t"<< measured_frame_rate<<std::endl;

    return measured_frame_rate*(1+th) < expected_frame_rate;

}
