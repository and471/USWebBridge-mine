#include <iostream>
#include <stdio.h>
#include <string>

#include "DNLImageSource.h"
#include <Modules/USStreamingCommon/DNLImage.h>
#include <vtkMetaImageWriter.h>
#include <vtkJPEGWriter.h>

void imaging_handler(DNLImage::Pointer imag);

int main(int argc, char *argv[])
{


    /****************************************************************************
     *                               ARGUMENT READ                              *
     ****************************************************************************/

    if (argc < 2){
        std::cout << "Not enough arguments"<<std::endl;
        std::cout << "Usage:"<<std::endl;
        std::cout << argv[0]<<" "<< "<folder>"<<std::endl;
        return -1;
    }

    std::string folder = argv[1];

    DNLImageSource *dnlIS = new DNLImageSource();
    dnlIS->SetFolder(folder);
    dnlIS->RegisterDNLImageHandler(&imaging_handler); /// This will start producing signals
    //boost::bind(&MyOtherClass::MyFunction, boost::ref(myobject), _1) // Use this above if your function is in other class

    dnlIS->GenerateImages();

    std::cout << "Press enter to terminate whenever you want!" << std::endl;
    std::string request;
    std::getline(std::cin, request);


    //server_active = false;
    std::cout << "Done!"<<std::endl;
    dnlIS->Stop();
    dnlIS->CleanThreads();


    /****************************************************************************
     *                                  CLEANUP                                 *
     ****************************************************************************/

    delete dnlIS;


    return 0;


}

/**
 * Here I just take the VTK image within the DNL image and print the spacing
 * as an example.
 */
void imaging_handler(DNLImage::Pointer imag){



    //double spacing[3];
    //imag->vtkImageLayers()[0]->GetSpacing(spacing);

    std::stringstream ss;
    ss<< "/home/ag09_local/data/iFIND/phantom/tmp/"<< "data_"<< imag->GetVTKImage()->GetDataDimension()
      <<"D_" << imag->GetDNLTimeStamp()<<".mhd";

      std::cout << "image: " << *(imag->GetVTKImage())<<std::endl;

        vtkSmartPointer<vtkMetaImageWriter>     writer  = vtkSmartPointer<vtkMetaImageWriter>::New();
        //vtkSmartPointer<vtkJPEGWriter>     writer  = vtkSmartPointer<vtkJPEGWriter>::New();
        writer->SetFileName(ss.str().data());
        writer->SetCompression(false);
        writer->SetInputData(imag->GetVTKImage());
        writer->Write();


}
