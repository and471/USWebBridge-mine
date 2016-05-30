#ifndef IMAGEMETADATA_H
#define IMAGEMETADATA_H

#include <string>

class ImageMetadata
{
public:
    double position[3] = {0,0,0};
    double orientation[4] = {0,0,0,0};

    // Equality operator
    inline bool operator == (const ImageMetadata &b) const {
        return std::equal(std::begin(position), std::end(position), std::begin(b.position)) &&
               std::equal(std::begin(orientation), std::end(orientation), std::begin(b.orientation));
    }

};

#endif // IMAGEMETADATA_H
