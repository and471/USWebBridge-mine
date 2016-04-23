#include "DNLFileImageSource.h"
#include <map>
#include <ctime>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <Modules/USStreamingCommon/DNLImageReader.h>

DNLFileImageSource::DNLFileImageSource(std::string &folder) {
    this->filenames.resize(0);
    this->stop_image_generation = false;
    this->thread = nullptr;

    // Find all image files in subfolders, and sort them in time order
    const PathType root(folder);
    get_all_files(root, ".mhd", this->filenames);
}

void DNLFileImageSource::start() {
    this->thread = new std::thread(&DNLFileImageSource::GenerateImagesThread, this);
}

void DNLFileImageSource::GenerateImagesThread() {

    std::vector<PathType>::const_iterator filename;
    for (filename = this->filenames.begin(); filename != this->filenames.end(); filename++){

        if (this->stop_image_generation){
            break;
        }

        std::string name = filename->string();

        DNLImageReader::Pointer reader = DNLImageReader::Pointer(new DNLImageReader());
        reader->SetFilename(name);
        reader->Read();

        DNLImage::Pointer image = reader->GetDNLImage();
        this->onImage(image);

        double framerate = image->acquisitionFrameRate()[0];
        double At = 50; // default value, ms
        if (framerate>0){
            At= 1.0/framerate*1000.0;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds((int) At));
    }
}

void DNLFileImageSource::stop() {
    this->stop_image_generation = true;
    if (this->thread->joinable()){
        this->thread->join();
    }

    delete this->thread;
}

/*
 * Return the filenames of all files that have the specified extension
 * in the specified directory and all subdirectories
 */
void DNLFileImageSource::get_all_files(const PathType& root, const std::string& ext, std::vector<PathType>& ret)
{
    if(!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root)) {
        return;
    }

    boost::filesystem::recursive_directory_iterator it(root);
    boost::filesystem::recursive_directory_iterator endit;

    while(it != endit) {
        if(boost::filesystem::is_regular_file(*it) && it->path().extension() == ext) {
            ret.push_back(it->path());
        }
        it++;
    }

    /// Sort the files in ascending order of modification
    std::sort(ret.begin(), ret.end(), mhd_file_sort());

}
