#ifndef IMAGEMETADATA_H
#define IMAGEMETADATA_H

#include <string>

class ImageMetadata
{
public:
    double position[3] = {0,0,0};
    // A quaternion, with the first 3 components representing the axis
    // the probe lies in, and the 4th representing the orientation around that axis
    double orientation[4] = {0,0,0,0};

    // Force in x,y,z direction (N) and 3 torques (Nm)
    double forces[6] = {0,0,0,0,0,0};

    // Size in mm which each pixel represents
    double spacing = 0;

    // Equality operator
    inline bool operator == (const ImageMetadata &b) const {
        return std::equal(std::begin(position), std::end(position), std::begin(b.position)) &&
               std::equal(std::begin(orientation), std::end(orientation), std::begin(b.orientation)) &&
               std::equal(std::begin(forces), std::end(forces), std::begin(b.forces)) &&
               spacing == b.spacing;
    }

};

#endif // IMAGEMETADATA_H
