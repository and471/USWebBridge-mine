#include <iostream>
#include <stdio.h>
#include <string>

#include <vtkSmartPointer.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkJPEGWriter.h>

#include "DNLImageSource.h"
#include "DNLFileImageSource.h"
#include <Modules/USStreamingCommon/DNLImage.h>
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

    DNLImageSource *dnlIS = new DNLFileImageSource(folder);
    dnlIS->connect(&imaging_handler); /// This will start producing signals
    //boost::bind(&MyOtherClass::MyFunction, boost::ref(myobject), _1) // Use this above if your function is in other class

    dnlIS->start();

    std::cout << "Press enter to terminate whenever you want!" << std::endl;
    std::string request;
    std::getline(std::cin, request);

    dnlIS->stop();

    //server_active = false;
    std::cout << "Done!"<<std::endl;


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
void imaging_handler(DNLImage::Pointer image){

    std::string outputFilename = "output.vtk";
    double spacing2[3];
    image->GetVTKImage(0)->GetSpacing(spacing2);
    vtkSmartPointer<vtkImageData> image_data =
            vtkSmartPointer<vtkImageData>::New();
    image_data->DeepCopy(image->GetVTKImage());
    int N = image_data->GetDataDimension();

    int *dims = new int[3];
    dims = image->GetVTKImage()->GetDimensions();

//    if (N !=2)
//        return;
      // Create a 100x100 image to save into the jpeg file
      int extent[6] = { 0, 99, 0, 99, 0, 0 };
      vtkSmartPointer<vtkImageCanvasSource2D> imageSource =
        vtkSmartPointer<vtkImageCanvasSource2D>::New();
      imageSource->SetExtent( extent );
      imageSource->SetScalarTypeToUnsignedChar(); // vtkJPEGWriter only accepts unsigned char input
      imageSource->SetNumberOfScalarComponents( 3 ); // 3 color channels: Red, Green and Blue

      // Fill the whole image with a blue background
      imageSource->SetDrawColor( 0, 127, 255 );
      imageSource->FillBox( extent[0], extent[1], extent[2], extent[3] );

      // Paint a 30x30 white square into the image
      imageSource->SetDrawColor( 255, 255, 255 );
      imageSource->FillBox( 40, 70, 20, 50 );


              vtkSmartPointer<vtkImageWriter> writer =
                vtkSmartPointer<vtkImageWriter>::New();
     // vtkSmartPointer<vtkJPEGWriter> writer =
      //  vtkSmartPointer<vtkJPEGWriter>::New();
      writer->SetFileName( outputFilename.c_str() );

      //writer->SetInputConnection( imageSource->GetOutputPort() );
      imageSource->Update();
      //writer->SetInputData(imageSource->GetOutput());
       writer->SetInputData(image->GetVTKImage(0));
      writer->Write();

    double spacing[3];
    image->GetVTKImage(0)->GetSpacing(spacing);

    std::cout << "ImageSpacing: " << spacing[0]<<", "<< spacing[1]<<std::endl;
/*

    vtkSmartPointer<vtkJPEGWriter> writer =
        vtkSmartPointer<vtkJPEGWriter>::New();
      writer->SetFileName("/home/andrew/Temp/hello.jpg");
      writer->SetInputData(image->GetVTKImage(0));
      writer->Write();
      */
}
