/**
* Convenience class that represent the current imaging parameters of an us acquisition system
*/
#ifndef USACQUISITIONPARAMETERS_H_
#define USACQUISITIONPARAMETERS_H_

#include "DNLImage.h"

class DNLUSAcquisitionParameters : public DNLImage{


public:

    typedef std::shared_ptr<DNLUSAcquisitionParameters> Pointer;

    typedef enum {NOCHANGE,DEPTH,WIDTH} PARAMETERCHANGE;

    using DNLImage::DNLImage;

    ~DNLUSAcquisitionParameters(){};


    bool hasDepthChanged(DNLImage::Pointer next);
    bool hasAFrameDropped(DNLImage::Pointer next, int layer=0);


private:



};

#endif // USACQUISITIONPARAMETERS_H_
