/**
* Convenience class that contains an image (itk or vtk) and other data from the TLO in a compact way
*/
#ifndef DNLImageWriter_H_
#define DNLImageWriter_H_



#include "DNLImage.h"
#include <memory>
#include <mutex>

inline char sep()
{
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}


class DNLImageWriter {

public:

    typedef std::shared_ptr<DNLImageWriter> Pointer;

public:
    DNLImageWriter();
    ~DNLImageWriter();

    void SetPath(std::string &Path);
    std::vector<std::string> GetFilename();
    std::vector<std::string> GetFullFilename();

    void Write(DNLImage::Pointer im);


private:

    DNLImage::Pointer m_image;
    std::vector<std::string> m_filename;
    std::string m_path;
    std::mutex m_writing_mutex;


};

#endif // DNLImageWriter_H_
