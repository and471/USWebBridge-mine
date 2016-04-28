#include "DNLImage.h"

#include <vtkImageImport.h>
#include <chrono> /// c++11
#include <Modules/Sensors/TimeManager.h>
#include <Modules/Sensors/ForceConversions.h>

#ifndef _WIN32
#include <dirent.h>
#else
#include <Modules/ProprietaryPhilips/DNLSimulatorCommon/Tools/dirent_win.h>
#endif

#include <memory> 
#include <vtkImageFlip.h>

#ifdef ITK_SUPPORT
#include <itkVTKImageToImageFilter.h>
#include <itkImageToVTKImageFilter.h>
#endif

#include <vtkInformation.h>

/// For tests only
#include <vtkMetaImageWriter.h>



DNLImage::DNLImage(){
    this->m_vtkImageLayers.resize(0);
    this->m_dnlLayerTimeTag.resize(0);
    this->m_forceTimestamp = std::string("-1");
    this->m_trackerTimestamp = std::string("-1");
    this->m_transducerTimestamp = std::string("-1");

    this->m_imageMode = ImageMode::TwoD;

    this->m_calibrationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    this->m_calibrationMatrix->Identity();
    this->m_trackerMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    this->m_trackerMatrix->Identity();
    this->m_transducerMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    this->m_transducerMatrix->Identity();

    this->m_reorientMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    this->m_reorientMatrix->Zero();
    this->m_reorientMatrix->Element[0][0] = 1;
    this->m_reorientMatrix->Element[2][1] = 1;
    this->m_reorientMatrix->Element[1][2] = -1;
    this->m_reorientMatrix->Element[3][3] = 1;

    this->m_ndimensions = -1;
    this->m_forceData.resize(0);
    this->m_trackerData.resize(0);

    // layer specific parameters
    this->m_acquisitionFrameRate.resize(0);
    this->m_transmissionFrameRate.resize(0);
    this->m_sectorWidth.resize(0);
    this->m_sectorAngle.resize(0);
    this->m_echoGain.resize(0);

    this->UpdateLocalTimestamp();

}



DNLImage::DNLImage(DNLImage::Pointer cSource) : DNLImage() {

    this->m_imageMode = cSource->m_imageMode;
    this->m_dnlTimestamp = cSource->m_dnlTimestamp;
    this->m_forceTimestamp = cSource->m_forceTimestamp;
    this->m_trackerTimestamp = cSource->m_trackerTimestamp;
    this->m_localTimestamp = cSource->m_localTimestamp;
    this->m_transducerTimestamp = cSource->m_transducerTimestamp;
    this->m_patientName = cSource->m_patientName;
    this->m_focusDepth = cSource->m_focusDepth;
    this->m_depthOfScanField = cSource->m_depthOfScanField;

    this->m_transducerNumber = cSource->m_transducerNumber;
    if (cSource->m_calibrationMatrix != nullptr){
        if (this->m_calibrationMatrix == nullptr) this->m_calibrationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        this->m_calibrationMatrix->DeepCopy(cSource->m_calibrationMatrix);
    }
    if (cSource->m_trackerMatrix != nullptr){
        if (this->m_trackerMatrix == nullptr) this->m_trackerMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        this->m_trackerMatrix->DeepCopy(cSource->m_trackerMatrix);
    }
    if (cSource->m_transducerMatrix != nullptr){
        if (this->m_transducerMatrix == nullptr) this->m_transducerMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        this->m_transducerMatrix->DeepCopy(cSource->m_transducerMatrix );
    }
    if (cSource->m_reorientMatrix != nullptr){
        if (this->m_reorientMatrix == nullptr) this->m_reorientMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        this->m_reorientMatrix->DeepCopy(cSource->m_reorientMatrix);
    }

    this->m_forceData = cSource->m_forceData;
    this->m_trackerData = cSource->m_trackerData;
    this->m_ndimensions = cSource->m_ndimensions;

    /// Layer stuff
    this->m_acquisitionFrameRate = cSource->m_acquisitionFrameRate;
    this->m_transmissionFrameRate = cSource->m_transmissionFrameRate;
    this->m_sectorWidth = cSource->m_sectorWidth;
    this->m_sectorAngle = cSource->m_sectorAngle;
    this->m_echoGain = cSource->m_echoGain;
    this->m_dnlLayerTimeTag= cSource->m_dnlLayerTimeTag;

    this->m_vtkImageLayers.resize(cSource->m_vtkImageLayers.size());
    for (int i=0; i<this->m_vtkImageLayers.size(); ++i){
        this->m_vtkImageLayers[i] = vtkSmartPointer<vtkImageData>::New();
        this->m_vtkImageLayers[i]->DeepCopy(cSource->m_vtkImageLayers[i]);
    }

    //std::vector< itkImageType3::Pointer > m_itkImageLayers;

}

DNLImage::DNLImage(std::vector< vtkSmartPointer<vtkImageData> > &vtkImageLayers,
                    MatrixType transducerMatrix, MatrixType trackerMatrix, MatrixType totalMatrix,MatrixType reorientMatrix,
                    DataStreamClientDataType &forces,
                    DataStreamClientDataType &position, long int dnlTimestamp,
                    std::vector<uint64_t> &dnlLayerTimeTagInt, long int localTimeStampInt,
                    long int trackerTimeStampInt,  long int forceTimeStampInt,
                    long int transducerTimeStampInt, std::vector<int> &acqfr, std::vector<int> txfr) : DNLImage(){


    //this->m_imageMode = cSource->m_imageMode;
    {
        std::stringstream ss;
        ss << dnlTimestamp;
        this->m_dnlTimestamp = ss.str();
    }
    {
        std::stringstream ss;
        ss << forceTimeStampInt;
        this->m_forceTimestamp = ss.str();
    }
    {
        std::stringstream ss;
        ss << trackerTimeStampInt;
        this->m_trackerTimestamp = ss.str();
    }
    {
        std::stringstream ss;
        ss << localTimeStampInt;
        this->m_localTimestamp = ss.str();
    }
    {
        std::stringstream ss;
        ss << transducerTimeStampInt;
        this->m_transducerTimestamp = ss.str();
    }
    //this->m_patientName = cSource->m_patientName;
    //this->m_focusDepth = cSource->m_focusDepth;
    //this->m_depthOfScanField = cSource->m_depthOfScanField;
    //this->m_transducerNumber = cSource->m_transducerNumber;
    //if (cSource->m_calibrationMatrix != nullptr){
    //    if (this->m_calibrationMatrix == nullptr) this->m_calibrationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    //    this->m_calibrationMatrix->DeepCopy();
    //}



    if (trackerMatrix != nullptr){
        if (this->m_trackerMatrix == nullptr) this->m_trackerMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        this->m_trackerMatrix->DeepCopy(trackerMatrix);
    }
    if (transducerMatrix != nullptr){
        if (this->m_transducerMatrix == nullptr) this->m_transducerMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        this->m_transducerMatrix->DeepCopy(transducerMatrix );
    }
    if (reorientMatrix != nullptr){
        if (this->m_reorientMatrix == nullptr) this->m_reorientMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        this->m_reorientMatrix->DeepCopy(reorientMatrix);
    }

    this->m_forceData = forces;
    this->m_trackerData = position;
    //this->m_ndimensions = cSource->m_ndimensions;

    /// Layer stuff
    this->m_acquisitionFrameRate = acqfr;
    this->m_transmissionFrameRate = txfr;
    //this->m_sectorWidth = cSource->m_sectorWidth;
    //this->m_sectorAngle = cSource->m_sectorAngle;
    //this->m_echoGain = cSource->m_echoGain;
    this->m_dnlLayerTimeTag= dnlLayerTimeTagInt;

    this->m_vtkImageLayers.resize(vtkImageLayers.size());
    for (int i=0; i<this->m_vtkImageLayers.size(); ++i){
        //this->m_vtkImageLayers[i] = vtkSmartPointer<vtkImageData>::New();
        this->m_vtkImageLayers[i] = vtkImageLayers[i];
    }



}



DNLImage::~DNLImage(){};


DNLImage::ImageMode DNLImage::imageMode() const
{
    return m_imageMode;
}

void DNLImage::setImageMode(const ImageMode &imageMode)
{
    m_imageMode = imageMode;
}

std::string DNLImage::dnlTimestamp() const
{
    return m_dnlTimestamp;
}

void DNLImage::setDnlTimestamp(const std::string &dnlTimestamp)
{
    m_dnlTimestamp = dnlTimestamp;
}

std::string DNLImage::patientName() const
{
    return m_patientName;
}

void DNLImage::setPatientName(const std::string &patientName)
{
    m_patientName = patientName;
}

int DNLImage::focusDepth() const
{
    return m_focusDepth;
}

void DNLImage::setFocusDepth(int focusDepth)
{
    m_focusDepth = focusDepth;
}

int DNLImage::depthOfScanField() const
{
    return m_depthOfScanField;
}

void DNLImage::setDepthOfScanField(int depthOfScanField)
{
    m_depthOfScanField = depthOfScanField;
}

int DNLImage::transducerNumber() const
{
    return m_transducerNumber;
}

void DNLImage::setTransducerNumber(int transducerNumber)
{
    m_transducerNumber = transducerNumber;
}

vtkSmartPointer<vtkMatrix4x4> DNLImage::calibrationMatrix() const
{
    return m_calibrationMatrix;
}

void DNLImage::setCalibrationMatrix(const vtkSmartPointer<vtkMatrix4x4> &calibrationMatrix)
{
    m_calibrationMatrix = calibrationMatrix;
}

int DNLImage::ndimensions() const
{
    return m_ndimensions;
}

void DNLImage::setNdimensions(int ndimensions)
{
    m_ndimensions = ndimensions;
}

std::vector<vtkSmartPointer<vtkImageData> > DNLImage::vtkImageLayers() const
{
    return m_vtkImageLayers;
}

void DNLImage::setVtkImageLayers(const std::vector<vtkSmartPointer<vtkImageData> > &vtkImageLayers)
{
    m_vtkImageLayers = vtkImageLayers;
}

std::vector<uint64_t> DNLImage::dnlLayerTimeTag() const
{
    return m_dnlLayerTimeTag;
}

void DNLImage::setDnlLayerTimeTag(const std::vector<uint64_t> &dnlLayerTimeTag)
{
    m_dnlLayerTimeTag = dnlLayerTimeTag;
}

std::vector<int> DNLImage::acquisitionFrameRate() const
{
    return m_acquisitionFrameRate;
}

void DNLImage::setAcquisitionFrameRate(const std::vector<int> &acquisitionFrameRate)
{
    m_acquisitionFrameRate = acquisitionFrameRate;
}

std::vector<int> DNLImage::transmissionFrameRate() const
{
    return m_transmissionFrameRate;
}

void DNLImage::setTransmissionFrameRate(const std::vector<int> &transmissionFrameRate)
{
    m_transmissionFrameRate = transmissionFrameRate;
}

std::vector<int> DNLImage::sectorWidth() const
{
    return m_sectorWidth;
}

void DNLImage::setSectorWidth(const std::vector<int> &sectorWidth)
{
    m_sectorWidth = sectorWidth;
}

std::vector<float> DNLImage::sectorAngle() const
{
    return m_sectorAngle;
}

void DNLImage::setSectorAngle(const std::vector<float> &sectorAngle)
{
    m_sectorAngle = sectorAngle;
}

std::vector<float> DNLImage::echoGain() const
{
    return m_echoGain;
}

void DNLImage::setEchoGain(const std::vector<float> &echoGain)
{
    m_echoGain = echoGain;
}


int DNLImage::GetNDimensions(){
    return this->m_ndimensions;
}


vtkSmartPointer<vtkMatrix4x4> DNLImage::GetCalibrationMatrix(){
    return this->m_calibrationMatrix;
}

void DNLImage::SetTrackerMatrix(vtkSmartPointer<vtkMatrix4x4> m){
    this->m_trackerMatrix = m;
}

vtkSmartPointer<vtkMatrix4x4> DNLImage::GetTrackerMatrix(){
    return this->m_trackerMatrix;
}

void DNLImage::SetTransducerMatrix(vtkSmartPointer<vtkMatrix4x4> m){
    this->m_transducerMatrix = m;
}

vtkSmartPointer<vtkMatrix4x4> DNLImage::GetTransducerMatrix(){
    return this->m_transducerMatrix;
}

vtkSmartPointer<vtkMatrix4x4> DNLImage::GetReorientMatrix(){
    return this->m_reorientMatrix;
}


vtkSmartPointer<vtkMatrix4x4> DNLImage::GetTotalMatrix(){

    vtkSmartPointer<vtkMatrix4x4> total = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkMatrix4x4::Multiply4x4(this->m_calibrationMatrix, this->m_trackerMatrix, total);
    vtkMatrix4x4::Multiply4x4(total, this->m_transducerMatrix, total);

    vtkSmartPointer<vtkMatrix4x4> Mimage = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkMatrix4x4::Multiply4x4(total, this->m_reorientMatrix, Mimage);


    return Mimage;
}



/**
* The int i says which part of the data retrieve
* For now the force format is timestamp transdurcerid v0 v1 v2 v3 v4 v5  [transducerid v0 v1 v2 ...]
*/
void DNLImage::SetForceData(DataStreamClientDataType &d, int i){

    if (d.size() < 1+(6+1) * (i + 1)){
        return;
    }

    this->m_forceData.resize(6);

    DataStreamClientDataType::iterator it;
    DataStreamClientDataType::const_iterator cit;
    for (cit = d.begin() + 1+(6+1) * i+1, it = this->m_forceData.begin(); it != this->m_forceData.end(); ++cit, ++it){
        *it = *cit;
    }
}



/**
 * @brief DNLImage::GetForceData
 * @param getForces if tru, get forces: fx fy fz tx ty tz, otherwise (default) get voltages
 * @return
 */
DNLImage::DataStreamClientDataType DNLImage::GetForceData(bool getForces){
    if (!getForces){
        return this->m_forceData;
    }

    DataStreamClientDataType force_data = ForceConversions::volts2forces(this->m_forceData, this->m_transducerNumber);

    return force_data;

}

/**
 * @brief DNLImage::SetTrackerData
 * @param d tracker data, for all transducers, in the form <N transducers> <ts0> <x y z> <q00 q01 q02 q03> <ts1> <x1 y1 z1> <....
 * @param i
 */
void DNLImage::SetTrackerData(DataStreamClientDataType &d, int i){

    int offset = 1 + i*(1 + 3 + 4) +1;  // ndevices, timestamp, data

    if (d.size() < offset +7 ){
        return;
    }

    this->m_trackerData.resize(7);

    DataStreamClientDataType::iterator it;
    DataStreamClientDataType::const_iterator cit;
    for (cit = d.begin() + offset, it = this->m_trackerData.begin(); it != this->m_trackerData.end(); ++cit, ++it){
        *it = *cit;
    }
}

DNLImage::DataStreamClientDataType DNLImage::GetTrackerData(){
    return this->m_trackerData;
}




std::string DNLImage::GetTrackerTimeStamp(){
    return this->m_trackerTimestamp;
}

std::string DNLImage::GetForceTimeStamp(){
    return this->m_forceTimestamp;
}


void DNLImage::SetTrackerTimeStamp(std::string & str){
    this->m_trackerTimestamp = str;
}

void DNLImage::SetForceTimeStamp(std::string & str){
    this->m_forceTimestamp = str;
}

void DNLImage::SetTransducerTimeStamp(std::string & str){
    this->m_transducerTimestamp = str;
}

std::string DNLImage::GetDNLTimeStamp(){
    return this->m_dnlTimestamp;
}

std::string DNLImage::GetLocalTimeStamp(){
    return this->m_localTimestamp;
}

std::string DNLImage::GetTransducerTimeStamp(){
    return this->m_transducerTimestamp;
}

long int DNLImage::GetTrackerTimeStampInt(){
    return atol(this->m_trackerTimestamp.c_str());
}

long int DNLImage::GetForceTimeStampInt(){
    return atol(this->m_forceTimestamp.c_str());
}

long int DNLImage::GetDNLTimeStampInt(){
    return atol(this->m_dnlTimestamp.c_str());
}

long int DNLImage::GetLocalTimeStampInt(){
    return atol(this->m_localTimestamp.c_str());
}

long int DNLImage::GetTransducerTimeStampInt(){
    return atol(this->m_transducerTimestamp.c_str());
}

vtkSmartPointer<vtkImageData> DNLImage::GetVTKImage(int layer) {
    if (this->m_vtkImageLayers.size() > layer){
        return this->m_vtkImageLayers[layer];
    }
    return nullptr;
}

#ifdef ITK_SUPPORT
/**
 * @brief DNLImage::GetITKImage
 * Convert the VTKImage to ITK and apply orientation
 * @param layer
 * @return
 */
DNLImage::itkImageType3::Pointer DNLImage::GetITKImage(int layer){

    /// TODO check that this is correct...

    itkImageType3::Pointer itkImage = DNLImage::VTKToITK(this->m_vtkImageLayers[layer]);

    // Apply orientation and origin
    itkImageType3::DirectionType orientation;
    itkImageType3::PointType offset;

    vtkSmartPointer<vtkMatrix4x4> totalMatrixVTK =  this->GetTotalMatrix();
    //vtkSmartPointer<vtkMatrix4x4> totalMatrixVTK =  this->GetCalibrationMatrix();
    //vtkSmartPointer<vtkMatrix4x4> totalMatrixVTK_i = totalMatrixVTK->Invert();

    for (int i=0; i<3;i++)
        for (int j=0; j<3;j++)
            orientation[i][j] = totalMatrixVTK->GetElement(i,j);


    itkImageType3::PointType origin_old = itkImage->GetOrigin();

    offset = totalMatrixVTK->MultiplyDoublePoint(origin_old.Begin());

    itkImage->SetDirection(orientation);
    itkImage->SetOrigin(offset);


    return itkImage;
}

DNLImage::itkImageType3::Pointer DNLImage::VTKToITK(vtkSmartPointer<vtkImageData> vtkImage){
    typedef itk::VTKImageToImageFilter<itkImageType3> VTKToITKType;
    VTKToITKType::Pointer convertor = VTKToITKType::New();

    convertor->SetInput( vtkImage);
    convertor->Update();

    itkImageType3::Pointer itkImage = convertor->GetOutput();
    return itkImage;
}

vtkSmartPointer<vtkImageData> DNLImage::ITKToVTK(DNLImage::itkImageType3::Pointer itkImage){

    typedef itk::ImageToVTKImageFilter<itkImageType3> ITKToVTKType;
    ITKToVTKType::Pointer convertor = ITKToVTKType::New();

    convertor->SetInput( itkImage);
    convertor->Update();

    vtkSmartPointer<vtkImageData> vtkImage = convertor->GetOutput();
    return vtkImage;
}
#endif

void DNLImage::UpdateLocalTimestamp(){
    std::stringstream ss;
    ss << TimeManager::GetLocalTimestamp();
    this->m_localTimestamp = ss.str();
}
