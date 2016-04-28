#ifndef PATIENTMETADATA_H
#define PATIENTMETADATA_H

#include <string>

class PatientMetadata
{
public:
    std::string name;

    // Equality operator
    inline bool operator == (const PatientMetadata &b) const {
        return (b.name == name);
    }

};

#endif // PATIENTMETADATA_H
