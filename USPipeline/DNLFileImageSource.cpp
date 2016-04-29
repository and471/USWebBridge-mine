#include "DNLFileImageSource.h"
#include <map>
#include <ctime>
#include <cstdlib>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <Modules/USStreamingCommon/DNLImageReader.h>

DNLFileImageSource::DNLFileImageSource(std::string &folder) {
    this->stop_image_generation = false;
    this->thread = nullptr;

    // Find all image files in subfolders, and sort them in time order
    PathType root(folder);
    get_mhd_files(root);
}

DNLFileImageSource::~DNLFileImageSource() {
    filenames.clear();
}

void DNLFileImageSource::start() {
    this->thread = new std::thread(&DNLFileImageSource::GenerateImagesThread, this);
}

void DNLFileImageSource::GenerateImagesThread() {
    // Generate images from each file, and when all are exhausted, start again
    while(true) {
        for (PathType filename : filenames) {

            if (this->stop_image_generation) {
                return;
            }

            std::string name = filename.string();

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
void DNLFileImageSource::get_mhd_files(PathType root)
{
    if(!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root)) {
        fprintf(stderr, "No such directory\n");
        exit(1);
        return;
    }

    boost::filesystem::directory_iterator end_iter;
    for (boost::filesystem::directory_iterator iter(root); iter != end_iter; iter++) {
        if (boost::filesystem::is_regular_file(*iter) && iter->path().extension() == ".mhd") {
            filenames.push_back(iter->path());
        }
    }

    /// Sort the files in ascending order of modification
    std::sort(filenames.begin(), filenames.end(), mhd_file_sort());
}
