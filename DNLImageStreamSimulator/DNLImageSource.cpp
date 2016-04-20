#include "DNLImageSource.h"
#include <map>
#include <ctime>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <Modules/USStreamingCommon/DNLImageReader.h>

DNLImageSource::DNLImageSource(){
    this->m_filenames.resize(0);
    this->m_extension = (".mhd");
    this->m_DNLImageSignalRegistered = false;
    this->m_endImageGeneration = false;
    this->m_generating_thread = nullptr;
}



DNLImageSource::~DNLImageSource(){

}


void DNLImageSource::RegisterDNLImageHandler(DNLImageHandlerSlotType slot)
{
    this->m_dnlImageSignal.connect(slot);
    this->m_DNLImageSignalRegistered = true;
}

void DNLImageSource::GenerateImages(){

    this->m_generating_thread = new std::thread(&DNLImageSource::GenerateImagesThread, this);


}

void DNLImageSource::GenerateImagesThread(){

    std::vector<PathType>::const_iterator cit;
    for (cit = this->m_filenames.begin(); cit != this->m_filenames.end(); ++cit){
        if (this->m_endImageGeneration){
            break;
        }

        std::string name = cit->string();

        std::cout << name<<std::endl;

        DNLImageReader::Pointer reader = DNLImageReader::Pointer(new DNLImageReader());
        reader->SetFilename(name);
        reader->Read();

        DNLImage::Pointer a = reader->GetDNLImage();

        this->m_dnlImageSignal(a);

        double framerate = a->acquisitionFrameRate()[0];
        double At = 50; // default value, ms
        if (framerate>0){
            At= 1.0/framerate*1000.0;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds((int) At));

    }
}

void DNLImageSource::CleanThreads(){
    if (this->m_generating_thread->joinable()){
        this->m_generating_thread->join();
    }
    delete this->m_generating_thread;
}

void DNLImageSource::Stop(){
    this->m_endImageGeneration = false;
}


bool DNLImageSource::SetFolder(std::string &folder){

    // Find all image files in subfolders, and sort them in time order
    const PathType root(folder) ;
    get_all_files(root, this->m_extension, this->m_filenames);

}



// return the filenames of all files that have the specified extension
// in the specified directory and all subdirectories
void DNLImageSource::get_all_files(const PathType& root, const std::string& ext, std::vector<PathType>& ret)
{
    if(!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root)) {
        return;
    }

    boost::filesystem::recursive_directory_iterator it(root);
    boost::filesystem::recursive_directory_iterator endit;

    while(it != endit)
    {
        if(boost::filesystem::is_regular_file(*it) && it->path().extension() == ext) {
            ret.push_back(it->path());
            //result_set.insert(result_set_t::value_type(boost::filesystem::last_write_time(it->path()), *it));
        }
        ++it;
    }

    /// Sort the files in ascending order of modification

    std::sort( ret.begin() , ret.end() ,mysort() );

}
