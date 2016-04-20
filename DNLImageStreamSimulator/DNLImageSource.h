#ifndef DNLIMAGESOURCE_H_
#define DNLIMAGESOURCE_H_

#include <string>
#include <vector>
#include <boost/filesystem/operations.hpp>
#include <sstream>
#include <thread>
#include <boost/signals2.hpp>
#include <Modules/USStreamingCommon/DNLImage.h>

class DNLImageSource {
public:

    /// Define signals
    typedef  boost::signals2::signal<void (DNLImage::Pointer )> DNLImageSignalType;
    typedef DNLImageSignalType::slot_type DNLImageHandlerSlotType;

    typedef boost::filesystem::path PathType;


    DNLImageSource();
    ~DNLImageSource();

    bool SetFolder(std::string &folder);


    void RegisterDNLImageHandler(DNLImageHandlerSlotType slot);

    void GenerateImages();
    void CleanThreads();
    void Stop();

    DNLImageSignalType m_dnlImageSignal;
    bool m_DNLImageSignalRegistered = false;

private:

    std::vector<PathType> m_filenames;
    std::string m_extension;
    std::thread *m_generating_thread;

    bool m_endImageGeneration;


    void GenerateImagesThread();

    static void get_all_files(const PathType& root, const std::string& ext, std::vector<PathType>& ret);

    /// sorting filenames by ascending order of timestamp, which is part of the file name
    struct mysort
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



#endif /// DNLIMAGESOURCE_H_
