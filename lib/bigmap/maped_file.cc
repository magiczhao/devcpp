#include "maped_file.h"
#include <string.h>
#include <errno.h>
#include "trace.h"

namespace devcpp
{

namespace bigmap
{

MappedFile::MappedFile(const string& filename, int start, int length) : _buffer(NULL), _size(0)
{
    init(filename, start, length);
}

MappedFile::~MappedFile()
{
    unmap();
}

bool MappedFile::unmap()
{
    if(!is_inited()){
        return true;
    }
    if(sync()){}
    else{
        trace("sync to file failed");
        return false;
    }

    if(munmap(_buffer, _size) == 0){
        _buffer = NULL;
        _size = 0;
        return true;
    }else{
        trace("munmap failed");
        return false;
    }
}

bool MappedFile::init(const string& filename, int start, int length)
{
    trace("init mmaped file, start:%d, length:%d", start, length);
    if(is_inited()){
        if(!unmap()){
            trace("unmap failed");
            return false;
        }
    }
    _filename = filename;
    int fd = open(_filename.c_str(), O_RDWR | O_CREAT, S_IRWXU | S_IRWXO);
    if(fd == -1){
        trace("open file failed:%s", strerror(errno));
        return false;
    }
    
    if(lseek(fd, start + length, SEEK_SET) == -1){
        close(fd);
        trace("seek failed:%s", strerror(errno));
        return false;
    }

    if(write(fd, "", 1) == -1){
        close(fd);
        trace("write to file failed:%s", strerror(errno));
        return false;
    }
    void* addr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, start);
    if(addr) {
        _buffer = addr;
        _size = length;
    }
    //trace("mapped file after init,buffer:%d, size:%d", _buffer, _size);
    
    close(fd);
    return _buffer != NULL;
}

bool MappedFile::sync(int start /*= 0*/, int size /*= 0*/)
{
    if(start < _size && start + size < _size){}
    else{
        return false;
    }

    if(size == 0) size = _size;
    char* addr = (char*)_buffer;
    if(start != 0){
        addr += start;
    }
    
    if(msync(addr, size, MS_SYNC) == 0){
        return true;
    }else{
        return false;
    }
    return false;
}

} //namespace shm
} //namespace devcpp

