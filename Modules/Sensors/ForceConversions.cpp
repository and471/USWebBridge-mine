#include "ForceConversions.h"

#include <iostream>

namespace ForceConversions{




double calibration_FT17105[6][6] = {{ -0.01977,   0.09926,  -0.06118,  -6.47648,   0.03413,   6.53854},
                                    {  0.17777,   8.02933,  -0.07961,  -3.75140,  -0.01064,  -3.80352},
                                    {  7.48665,   0.20823,   7.59437,   0.00236,   7.40519,   0.06589},
                                    { -0.00796,  49.02024,  41.15754, -22.68953, -42.22511, -23.63697},
                                    {-47.29680,  -1.75978,  25.58314,  39.62147,  24.99215, -39.72212},
                                    {  0.86342,  28.99397,   0.29133,  27.94167,   0.22638,  29.15076}};

double calibration_FT17106[6][6] = {{  0.01192,  -0.01139,   0.06801,  -6.59233,   0.03276,   6.46263},
                                    { -0.03088,   8.13213,  -0.02303,  -3.71232,  -0.09691,  -3.79986},
                                    {  7.35501,   0.45629,   7.47887,   0.30165,   7.44087,   0.32609},
                                    { -1.05470,  49.55016,  41.00417, -20.80677, -42.31678, -25.08276},
                                    {-47.44815,  -2.65583,  23.68215,  41.13938,  25.70873, -38.35560},
                                    { -0.60145,  30.41434,   0.05348,  29.22004,   0.32728,  28.66556}};


ArrayType volts2forces(ArrayType &volts, int sensor){
    ArrayType forces;
    if (volts.size()<=0){
        return forces;
    }
    for (int i=0; i< CHANNELS_PER_DEVICE; i++){
        forces[i] = 0;
        for (int j=0; j<CHANNELS_PER_DEVICE; j++){
            switch (sensor){
            case 0:
                forces[i] += calibration_FT17105[i][j]*volts[j];
                break;
            case 1:
                forces[i] += calibration_FT17106[i][j]*volts[j];
                break;
            default:
                std::cerr << "ERROR:: Only sensors 1 and 2 are implemented"<<std::endl;
            }
        }
    }

    return forces;
}

std::vector<double>  volts2forces(std::vector<double> &volts, int sensor){
    std::vector<double> forces(volts.size());
    if (volts.size()<=0){
        return forces;
    }

    for (int i=0; i< CHANNELS_PER_DEVICE; i++){
        forces[i] = 0;
        for (int j=0; j<CHANNELS_PER_DEVICE; j++){
            switch (sensor){
            case 0:
                forces[i] += calibration_FT17105[i][j]*volts[j];
                break;
            case 1:
                forces[i] += calibration_FT17106[i][j]*volts[j];
                break;
            default:
                std::cerr << "ERROR:: Only sensors 1 and 2 are implemented"<<std::endl;
            }
        }
    }

    return forces;
}


}
