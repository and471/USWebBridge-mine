#ifndef FORCECONVERSIONS_H_
#define FORCECONVERSIONS_H_

#include <array>
#include <vector>

namespace ForceConversions{

static const int CHANNELS_PER_DEVICE=6;


typedef std::array<double, CHANNELS_PER_DEVICE> ArrayType;

/****************************************************************************
*                               VOLTS TO FORCES                             *
****************************************************************************/

ArrayType volts2forces(ArrayType &volts, int sensor=0);
std::vector<double>  volts2forces(std::vector<double> &volts, int sensor=0);




}

#endif // FORCECONVERSIONS_H_
