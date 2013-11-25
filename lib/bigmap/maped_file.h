#ifndef _DEVCPP_LIB_SHM_MAPED_FILE_H
#define _DEVCPP_LIB_SHM_MAPED_FILE_H

#include <string>
using namespace std;

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace devcpp
{
namespace bigmap
{

class MappedFile
{
    public:
        MappedFile() : _filename(""), _buffer(NULL), _size(0){}
        MappedFile(const string& filename, int start, int length);
        ~MappedFile();
        bool init(const string& filename, int start, int length);
        bool sync(int start = 0, int size = 0);
        bool is_inited() const {return _buffer != NULL;}
        bool unmap();
        void* buffer() {return _buffer;}
        int size() const {return _size;}
        const string& filename() const {return _filename;}

        template<typename T>
        T* value(long position){return (T*)((char*)_buffer + position);}
    private:
        MappedFile(const MappedFile& mf){}
        MappedFile& operator=(const MappedFile& mf) {return *this;}

        int file_size();
    private:
        string  _filename;
        void*   _buffer;
        int     _size;
};

} //namespace shm

} //namespace devcpp
#endif
