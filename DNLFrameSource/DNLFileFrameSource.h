#ifndef DNLFILEFRAMESOURCE_H_
#define DNLFILEFRAMESOURCE_H_

#include <string>
#include <vector>
#include <boost/filesystem/operations.hpp>
#include <sstream>
#include <thread>
#include <atomic>
#include <boost/signals2.hpp>
#include <Modules/USStreamingCommon/DNLImage.h>
#include "DNLFrameSource.h"

class DNLFileFrameSource : public DNLFrameSource {
public:

    typedef boost::filesystem::path PathType;

    DNLFileFrameSource(std::string &folder);
    ~DNLFileFrameSource();

    void start();
    void stop();

private:

    std::vector<PathType> filenames;
    std::thread *thread;

    std::atomic_bool stop_image_generation;

    void GenerateImagesThread();

    void get_mhd_files(PathType root);

    /// sorting filenames by ascending order of timestamp, which is part of the file name
    struct mhd_file_sort
    {
        template <typename T>
        T st2num ( const std::string &Text )
        {
            std::stringstream ss(Text);
            T result;
            return ss >> result ? result : 0;
        }


        bool operator ()(const PathType & a,const PathType & b)
        {

            int idx1 = a.filename().string().find("D_t");
            int idx2 = b.filename().string().find("D_t");

            std::string stringa = a.filename().string().substr(idx1+3,13);
            std::string stringb = b.filename().string().substr(idx2+3,13);

            uint64_t int1 = st2num<uint64_t>(stringa);
            uint64_t int2 = st2num<uint64_t>(stringb);

            return int1 < int2;

        }
    };

};



#endif /// DNLFILEFRAMESOURCE_H_
