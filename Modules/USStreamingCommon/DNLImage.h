/**
* Convenience class that contains an image (itk or vtk) and other data from the TLO in a compact way
*/
#ifndef DNLImage_H_
#define DNLImage_H_

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <boost/integer.hpp>
#include <vector>
#include <sstream>

//#include <Modules/DataStreaming/DataStreamClient.h>

#ifdef ITK_SUPPORT
    #include <itkImage.h>
    #include <itkMatrix.h>
#endif

#include <memory>
#include <list>

class DNLImage {

public:

    typedef std::vector<double> DataStreamClientDataType;

    typedef vtkSmartPointer<vtkMatrix4x4> MatrixType;
    typedef std::shared_ptr<DNLImage> Pointer;
    typedef std::list<DNLImage::Pointer> DNLImageBufferType;

    enum class ImageMode { TwoD, ThreeDCartesian, ThreeDFrustum };
    enum class ImageFormat {
    #ifdef ITK_SUPPORT
        ITK,
    #endif
        VTK
    };
    typedef uint8_t Byte;

    #ifdef ITK_SUPPORT
    typedef itk::Image<Byte, 3> itkImageType3;
    #endif
    
    bool m_debug;

public:
    DNLImage();
    DNLImage(DNLImage::Pointer cSource);
    DNLImage(std::vector< vtkSmartPointer<vtkImageData> > &vtkImageLayers,
                        MatrixType transducerMatrix, MatrixType trackerMatrix, MatrixType totalMatrix,MatrixType reorientMatrix,
                        DataStreamClientDataType &forces,
                        DataStreamClientDataType &position, long int dnlTimestamp,
                        std::vector<uint64_t> &dnlLayerTimeTagInt, long int localTimeStampInt,
                        long int trackerTimeStampInt,  long int forceTimeStampInt,
                        long int transducerTimeStampInt, std::vector<int> &acqfr, std::vector<int> txfr);

    ~DNLImage();

    ImageMode GetImageMode(){ return this->m_imageMode; }
    vtkSmartPointer<vtkImageData> GetVTKImage(int layer = 0);

    #ifdef ITK_SUPPORT
    itkImageType3::Pointer GetITKImage(int layer=0);
    static itkImageType3::Pointer VTKToITK(vtkSmartPointer<vtkImageData> vtkImage);
    static vtkSmartPointer<vtkImageData> ITKToVTK(itkImageType3::Pointer itkImage);
    #endif

    std::string GetDNLTimeStamp();
    std::string GetLocalTimeStamp();
    std::string GetTrackerTimeStamp();
    std::string GetForceTimeStamp();
    std::string GetTransducerTimeStamp();

    long int GetDNLTimeStampInt();
    long int GetLocalTimeStampInt();
    long int GetTrackerTimeStampInt();
    long int GetForceTimeStampInt();
    long int GetTransducerTimeStampInt();

    void SetTrackerTimeStamp(std::string & str);
    void SetForceTimeStamp(std::string & str);
    void SetTransducerTimeStamp(std::string & str);
    int GetNDimensions();

    vtkSmartPointer<vtkMatrix4x4> GetCalibrationMatrix();
    void SetTrackerMatrix(vtkSmartPointer<vtkMatrix4x4> m);
    vtkSmartPointer<vtkMatrix4x4> GetTrackerMatrix();
    void SetTransducerMatrix(vtkSmartPointer<vtkMatrix4x4> m);
    vtkSmartPointer<vtkMatrix4x4> GetTransducerMatrix();
    vtkSmartPointer<vtkMatrix4x4> GetTotalMatrix();
    vtkSmartPointer<vtkMatrix4x4> GetReorientMatrix();
    void SetForceData(DataStreamClientDataType &d, int i=0);
    DataStreamClientDataType GetForceData(bool getForces =false);
    void SetTrackerData(DataStreamClientDataType &d, int i=0);
    DataStreamClientDataType GetTrackerData();


    // layer stuff

    ImageMode imageMode() const;
    void setImageMode(const ImageMode &imageMode);

    std::string dnlTimestamp() const;
    void setDnlTimestamp(const std::string &dnlTimestamp);

    std::string patientName() const;
    void setPatientName(const std::string &patientName);

    int focusDepth() const;
    void setFocusDepth(int focusDepth);

    int depthOfScanField() const;
    void setDepthOfScanField(int depthOfScanField);

    int transducerNumber() const;
    void setTransducerNumber(int transducerNumber);

    vtkSmartPointer<vtkMatrix4x4> calibrationMatrix() const;
    void setCalibrationMatrix(const vtkSmartPointer<vtkMatrix4x4> &calibrationMatrix);

    int ndimensions() const;
    void setNdimensions(int ndimensions);

    std::vector<vtkSmartPointer<vtkImageData> > vtkImageLayers() const;
    void setVtkImageLayers(const std::vector<vtkSmartPointer<vtkImageData> > &vtkImageLayers);

    std::vector<uint64_t> dnlLayerTimeTag() const;
    void setDnlLayerTimeTag(const std::vector<uint64_t> &dnlLayerTimeTag);

    std::vector<int> acquisitionFrameRate() const;
    void setAcquisitionFrameRate(const std::vector<int> &acquisitionFrameRate);

    std::vector<int> transmissionFrameRate() const;
    void setTransmissionFrameRate(const std::vector<int> &transmissionFrameRate);

    std::vector<int> sectorWidth() const;
    void setSectorWidth(const std::vector<int> &sectorWidth);

    std::vector<float> sectorAngle() const;
    void setSectorAngle(const std::vector<float> &sectorAngle);

    std::vector<float> echoGain() const;
    void setEchoGain(const std::vector<float> &echoGain);

    std::string m_patientName;

private:

    std::vector< vtkSmartPointer<vtkImageData> > m_vtkImageLayers;
    ImageMode m_imageMode;

    /// tlo fancy stuff
    std::string m_dnlTimestamp;
    std::string m_localTimestamp;
    std::string m_trackerTimestamp;
    std::string m_forceTimestamp;
    std::string m_transducerTimestamp;


    int m_ndimensions;
    int m_transducerNumber;
    int m_depthOfScanField;
    int m_focusDepth;


    vtkSmartPointer<vtkMatrix4x4> m_transducerMatrix;
    vtkSmartPointer<vtkMatrix4x4> m_calibrationMatrix;
    vtkSmartPointer<vtkMatrix4x4> m_trackerMatrix;
    vtkSmartPointer<vtkMatrix4x4> m_reorientMatrix;
    DataStreamClientDataType m_forceData;
    DataStreamClientDataType m_trackerData;

    /// layer specific stuff

    std::vector<int> m_sectorWidth;
    std::vector<float> m_sectorAngle;
    std::vector<float> m_echoGain;
    std::vector<uint64_t> m_dnlLayerTimeTag;
    std::vector<int> m_acquisitionFrameRate;
    std::vector<int> m_transmissionFrameRate;

    void UpdateLocalTimestamp();



};

#endif // DNLImage_H_
