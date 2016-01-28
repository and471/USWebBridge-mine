/**
* Convenience class that contains an image (itk or vtk) and other data from the TLO in a compact way
*/
#ifndef DNLIMAGEREADER_H_
#define DNLIMAGEREADER_H_


#include <memory>
#include "DNLImage.h"
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>

class DNLImageReader {

public:

    typedef struct {
        std::string ObjectType;
        int NDims =2;
        bool BinaryData= true;
        bool BinaryDataByteOrderMSB =false;
        bool CompressedData = false;
        double TransformMatrix[9]={1,0,0, 0,1,0, 0,0,1};
        double Offset[3] ={0,0,0};
        double CenterOfRotation[3]={0,0,0};
        double ElementSpacing[3]={1,1,1};
        int DimSize[3]={0,0,0};
        std::string AnatomicalOrientation="???";
        std::string ElementType = "MET_UCHAR";
        std::string ElementDataFile ="";
    } ImageHeader;

    typedef std::shared_ptr<DNLImageReader> Pointer;
    typedef vtkSmartPointer<vtkMatrix4x4> MatrixType;

public:
    DNLImageReader();
    ~DNLImageReader();

    void SetFilename(std::string &filename);
    void SetFilename(const char *filename);
    void Read();

    DNLImage::Pointer GetDNLImage();




private:

        std::string m_filename;
        DNLImage::Pointer m_image;

        static vtkSmartPointer<vtkImageData> ReadFromFile(std::string &filename);

};

#endif // DNLIMAGEREADER_H_
