#include "DNLImageWriter.h"

#include <vtkMetaImageWriter.h>

#include <boost/filesystem.hpp>

DNLImageWriter::DNLImageWriter(){
    this->m_image = nullptr;
    this->m_filename.resize(0);
}

DNLImageWriter::~DNLImageWriter(){};

std::vector<std::string> DNLImageWriter::GetFilename(){
    return this->m_filename;
}

std::vector<std::string> DNLImageWriter::GetFullFilename(){

    std::vector <std::string> full_filenames(this->m_filename.size());

    std::vector<std::string>::const_iterator cit;
    std::vector<std::string>::iterator it;
    for (cit = this->m_filename.begin(), it = full_filenames.begin(); cit != this->m_filename.end(); ++cit, ++it){
        *it = this->m_path + sep() + *cit;
    }

    return full_filenames;
}

void DNLImageWriter::SetPath(std::string &Path){
    this->m_path = Path;
}

void DNLImageWriter::Write(DNLImage::Pointer im){


    int nlayers = im->vtkImageLayers().size();

    this->m_filename.resize(nlayers);

    for (int currentLayer = 0; currentLayer < nlayers; currentLayer++){
        if (im->GetVTKImage(currentLayer) == nullptr){
            /// All layers must be good
            return;
        }

        std::ostringstream ss_transducer_number;
        ss_transducer_number << im->transducerNumber();

        std::ostringstream ss_ndimensions;
        ss_ndimensions << im->GetNDimensions();

        std::ostringstream ss_layer;
        ss_layer << currentLayer;

        /// Make a subfolder with the label if it does not exist

        {/// go hierarchically until the folder is successfully created
            std::vector<boost::filesystem::path> path_stack;
            boost::filesystem::path newpath(this->m_path.data());
            while (true){
                if (!boost::filesystem::exists(newpath)){
                    path_stack.push_back(newpath);
                    newpath  = newpath.parent_path();
                } else {
                    break;
                }
            }
            for (int i = 0; i< path_stack.size(); i++){
                boost::filesystem::create_directory(path_stack[path_stack.size()-i-1]);
            }
        }

        std::string filename_infix =  std::string("image")
                + std::string("_transducer") + ss_transducer_number.str()
                + std::string("_") + ss_ndimensions.str() + std::string("D") //+std::string("_frame") + ss_frame.str()
                + std::string("_t") + im->GetDNLTimeStamp() //+ ss_time.str() /// Using timestamp from DNL
                + std::string("_layer") + ss_layer.str();

        std::string filename_base = this->m_path + sep() + filename_infix;

        vtkSmartPointer<vtkMetaImageWriter> writer = vtkSmartPointer<vtkMetaImageWriter>::New();
        writer->SetInputData(im->GetVTKImage(currentLayer));
        writer->SetFileName((filename_base + ".mhd").c_str());
        writer->SetRAWFileName((filename_base + ".raw").c_str());
        writer->SetCompression(false);
        this->m_writing_mutex.lock();
        writer->Write();

        /// Append stuff to header ----------------------
        fstream myfile;

        this->m_filename[currentLayer] = filename_infix + std::string(".mhd");

        myfile.open((filename_base + ".mhd"), fstream::app | fstream::out);

        /// Append to the header pressure and position

        /// Save the calibration matrix
        {
            myfile << "#TRANSDUCERMATRIX\t";
            for (int ii = 0; ii < 4; ii++){
                for (int jj = 0; jj < 4; jj++){
                    myfile << im->GetTransducerMatrix()->GetElement(ii, jj) << " ";
                }
            }
            myfile << std::endl;
        }
        /// Save the tracker matrix
        {
            myfile << "#TRACKERMATRIX\t";
            for (int ii = 0; ii < 4; ii++){
                for (int jj = 0; jj < 4; jj++){
                    myfile << im->GetTrackerMatrix()->GetElement(ii, jj) << " ";
                }
            }
            myfile << std::endl;
        }
        /// Save the reorient matrix
        {
            myfile << "#REORIENTMATRIX\t";
            for (int ii = 0; ii < 4; ii++){
                for (int jj = 0; jj < 4; jj++){
                    myfile << im->GetReorientMatrix()->GetElement(ii, jj) << " ";
                }
            }
            myfile << std::endl;
        }
        /// Save the total matrix
        {
            myfile << "#TOTALMATRIX\t";
            for (int ii = 0; ii < 4; ii++){
                for (int jj = 0; jj < 4; jj++){
                    myfile << im->GetTotalMatrix()->GetElement(ii, jj) << " ";
                }
            }
            myfile << std::endl;
        }

        DNLImage::DataStreamClientDataType forceData = im->GetForceData(); // if we want to save volts
        //DataStreamClientDataType forceData = im->GetForceData(true); // If we want to save forces
        if (forceData.size()>0){
            fstream myfile;
            myfile.open((filename_base + ".mhd"), fstream::app | fstream::out);
            myfile << "#FORCE";
            DNLImage::DataStreamClientDataType::const_iterator citf;
            for (citf = forceData.begin(); citf != forceData.end(); ++citf){
                myfile << "\t" << *citf;
            }
            myfile << std::endl;
        }

        DNLImage::DataStreamClientDataType trackerData = im->GetTrackerData();
        if (trackerData.size() >0){
            myfile << "#POSITION";
            DNLImage::DataStreamClientDataType trackerData = im->GetTrackerData();
            DNLImage::DataStreamClientDataType::const_iterator citp;
            for (citp = trackerData.begin(); citp != trackerData.end(); ++citp){
                myfile << "\t" << *citp;
            }
            myfile << std::endl;
            myfile << "#TIMESTAMP_TRACKER\t" << im->GetTrackerTimeStamp() << std::endl;
        }
        // Save timestamps
        {
            myfile << "#TIMESTAMP_LOCAL\t" << im->GetLocalTimeStamp() << std::endl;
            myfile << "#TIMESTAMP_DNL\t" << im->GetDNLTimeStamp() << std::endl;
            myfile << "#TIMESTAMP_DNLLAYERTIMELAG\t" << im->dnlLayerTimeTag()[0]<<std::endl;

            // save framerate
            myfile << "#ACQFR\t" << im->acquisitionFrameRate()[0]<<std::endl;
            myfile << "#TXFR\t" << im->transmissionFrameRate()[0]<<std::endl;

        }

        /*
            /// Save the perceived force
            {
            fstream myfile;
            myfile.open((filename_base + ".mhd"), fstream::app | fstream::out);
            myfile << "#PERCEIVEDF\t" << this->m_viewer->GetPerceivedForce() << std::endl;
            myfile.close();
            }
            */

        myfile.close();
        this->m_writing_mutex.unlock();
    }


}
