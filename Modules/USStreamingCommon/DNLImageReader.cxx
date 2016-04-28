#include "DNLImageReader.h"
#include <fstream>
#include <vtkImageImport.h>
DNLImageReader::DNLImageReader(){

}



DNLImageReader::~DNLImageReader(){};


void DNLImageReader::SetFilename(const char *filename){
    this->m_filename = std::string(filename);

}

void DNLImageReader::SetFilename(std::string &filename){
    this->m_filename = filename;

}

void DNLImageReader::Read(){



    std::vector<std::string> layer_filenames;

    // find out the number of layers
    layer_filenames.push_back(this->m_filename);
    int nlayers = 1; // FIXME search in current folder for other images with same name but different layer

    /// Data to be passed to the constuctor ------------------------

    MatrixType transducerMatrix;
    MatrixType trackerMatrix;
    MatrixType totalMatrix;
    MatrixType reorientMatrix;
    long int dnlTimestamp;
    long int localTimeStampInt;
    long int trackerTimeStampInt;
    long int forceTimeStampInt;
    long int transducerTimeStampInt;
    DNLImage::DataStreamClientDataType forces;
    DNLImage::DataStreamClientDataType position;

    std::vector<vtkSmartPointer<vtkImageData> > vtkImageLayers(nlayers);
    std::vector<uint64_t> dnlLayerTimeTagInt(nlayers);
    std::vector<int> acqfr(nlayers);
    std::vector<int> txfr(nlayers);

    ImageHeader header;
    /// Iterate over layers ------------------------
    for (int i=0; i<nlayers; i++){

        /// Read image data ///
        vtkImageLayers[i] = ReadFromFile(layer_filenames[i], &header);

        /// Read meta data ///

        fstream myfile;
        myfile.open(layer_filenames[i], fstream::in);

        char output[100];

        if (myfile.is_open()) {
            while (!myfile.eof()) {
                myfile >> output;
                if (!strcmp(output, "#TRANSDUCERMATRIX")){
                    const int NVALS = 16;
                    double value[NVALS];
                    for (int j =0; j<NVALS; j++){
                        myfile >> value[j];
                    }
                    transducerMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
                    transducerMatrix->DeepCopy(value);

                } else if (!strcmp(output, "#TRACKERMATRIX")){
                    const int NVALS = 16;
                    double value[NVALS];
                    for (int j =0; j<NVALS; j++){
                        myfile >> value[j];
                    }
                    trackerMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
                    trackerMatrix->DeepCopy(value);
                } else if (!strcmp(output, "#REORIENTMATRIX")){
                    const int NVALS = 16;
                    double value[NVALS];
                    for (int j =0; j<NVALS; j++){
                        myfile >> value[j];
                    }
                    reorientMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
                    reorientMatrix->DeepCopy(value);
                } else if (!strcmp(output, "#TOTALMATRIX")){
                    const int NVALS = 16;
                    double value[NVALS];
                    for (int j =0; j<NVALS; j++){
                        myfile >> value[j];
                    }
                    totalMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
                    totalMatrix->DeepCopy(value);
                } else if (!strcmp(output, "#TIMESTAMP_LOCAL")){
                    myfile >> localTimeStampInt;
                } else if (!strcmp(output, "#TIMESTAMP_DNL")){
                    myfile >> dnlTimestamp;
                } else if (!strcmp(output, "#TIMESTAMP_DNLLAYERTIMELAG")){
                    myfile >>  dnlLayerTimeTagInt[i];
                } else if (!strcmp(output, "#TIMESTAMP_TRACKER")){
                    myfile >> trackerTimeStampInt;
                } else if (!strcmp(output, "#ACQFR")){
                    myfile >> acqfr[i];
                } else if (!strcmp(output, "#TXFR")){
                    myfile >> txfr[i];
                } else if (!strcmp(output, "#FORCE")){
                    const int NVALS = 6;
                    double value;
                    forces.reserve(NVALS);
                    for (int j =0; j<NVALS; j++){
                        myfile >> value;
                        forces.push_back(value);
                    }
                } else if (!strcmp(output, "#POSITION")){
                    const int NVALS = 7;
                    double value;
                    position.reserve(NVALS);
                    for (int j =0; j<NVALS; j++){
                        myfile >> value;
                        position.push_back(value);
                    }
                }
            }
        }
        myfile.close();
    }

    this->m_image = DNLImage::Pointer(new DNLImage(vtkImageLayers,
                                                   transducerMatrix, trackerMatrix, totalMatrix, reorientMatrix,
                                                   forces, position,
                                                   dnlTimestamp, dnlLayerTimeTagInt, localTimeStampInt, trackerTimeStampInt, forceTimeStampInt, transducerTimeStampInt,
                                                   acqfr, txfr)
                                      );
    this->m_image->setNdimensions(header.NDims);

}

DNLImage::Pointer DNLImageReader::GetDNLImage(){

    return this->m_image;
}


vtkSmartPointer<vtkImageData> DNLImageReader::ReadFromFile(std::string &filename, ImageHeader* header){

    fstream myfile;
    myfile.open(filename, fstream::in);

    std::string field;

    if (myfile.is_open()) {
        while (!myfile.eof()) {
            myfile >> field;
            if (!strcmp(field.data(), "ObjectType")){
                myfile >> field>>header->ObjectType;
            } else if (!strcmp(field.data(), "NDims")){
                myfile >> field>>header->NDims;
            } else if (!strcmp(field.data(), "BinaryData")){
                std::string boolFlag;
                myfile >> field>>boolFlag;
                header->BinaryData = false;
                if (!strcmp(boolFlag.data(), "True")){
                    header->BinaryData = true;
                }
            } else if (!strcmp(field.data(), "BinaryDataByteOrderMSB")){
                std::string boolFlag;
                myfile >> field>>boolFlag;
                header->BinaryDataByteOrderMSB = false;
                if (!strcmp(boolFlag.data(), "True")){
                    header->BinaryDataByteOrderMSB = true;
                }
            } else if (!strcmp(field.data(), "CompressedData")){
                std::string boolFlag;
                myfile >> field>>boolFlag;
                header->CompressedData = false;
                if (!strcmp(boolFlag.data(), "True")){
                    header->CompressedData = true;
                }
            } else if (!strcmp(field.data(), "TransformMatrix")){
                myfile >> field;
                for (int i=0; i<header->NDims*header->NDims; i++){
                    myfile >>header->TransformMatrix[i];
                }
            } else if (!strcmp(field.data(), "Offset")){
                myfile >> field;
                for (int i=0; i<header->NDims; i++){
                    myfile >>header->Offset[i];
                }
            } else if (!strcmp(field.data(), "CenterOfRotation")){
                myfile >> field;
                for (int i=0; i<header->NDims; i++){
                    myfile >>header->CenterOfRotation[i];
                }
            } else if (!strcmp(field.data(), "ElementSpacing")){
                myfile >> field;
                for (int i=0; i<header->NDims; i++){
                    myfile >>header->ElementSpacing[i];
                }
            } else if (!strcmp(field.data(), "DimSize")){
                myfile >> field;
                for (int i=0; i<header->NDims; i++){
                    myfile >>header->DimSize[i];
                }
            } else if (!strcmp(field.data(), "AnatomicalOrientation")){
                myfile >> field;
                myfile >>header->AnatomicalOrientation;
            } else if (!strcmp(field.data(), "ElementType")){
                myfile >> field;
                myfile >>header->ElementType;
            } else if (!strcmp(field.data(), "ElementDataFile")){
                myfile >> field;
                myfile >>header->ElementDataFile;
            }
        }
    }
    myfile.close();


    vtkSmartPointer<vtkImageData> image;

    /// Read raw data
    if (!header->CompressedData){

        int length = 1;
        for (int i=0; i< header->NDims; i++){
            length *= header->DimSize[i];
        }

        int idx = filename.find_last_of('/');
        std::string path = filename.substr(0,idx);

        int NBYTES;
        if (!strcmp(header->ElementType.data(),"MET_UCHAR")){
            NBYTES = length*sizeof(char);
            char * buffer = new char[NBYTES];
            std::ifstream is(path + std::string("/") +header->ElementDataFile, ios::binary );
            if (is) {
                // read data as a block:
                is.read (buffer, length);
                is.close();
            }

            /// create vtkImage
            vtkSmartPointer<vtkImageImport> importer = vtkSmartPointer<vtkImageImport>::New();
            importer->SetDataSpacing(header->ElementSpacing);
            importer->SetDataOrigin(header->Offset);
            importer->SetWholeExtent(0, header->DimSize[0] - 1, 0, header->DimSize[1] - 1, 0, header->DimSize[2] - 1);
            importer->SetDataExtentToWholeExtent();
            importer->SetDataScalarTypeToUnsignedChar();
            importer->SetNumberOfScalarComponents(1);

            importer->SetImportVoidPointer(buffer, 1);
            importer->Update();

            image = vtkSmartPointer<vtkImageData>::New();
            image->DeepCopy(importer->GetOutput());

            delete [] buffer;  // If deleted, problem when writing!!!


        }
    } else {
        // Not implemented for compressed data yet
        std::cout << "Not implemented for compressed data yet"<<std::endl;
    }

    return image;

}

