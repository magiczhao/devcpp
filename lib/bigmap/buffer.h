#ifndef _DEVCPP_LIB_BIGMAP_BUFFER_H
#define _DEVCPP_LIB_BIGMAP_BUFFER_H

#include <string.h>
namespace devcpp
{
namespace bigmap
{

class Buffer
{
    public:
        Buffer() : _buffer(NULL), _size(0)
        {}

        Buffer(char* buffer, int size) : _buffer(buffer), _size(size)
        {}

        Buffer(char* buffer) : _buffer(buffer), _size(0)
        {
            _size = strlen(buffer);
        }
        ~Buffer(){}

        Buffer(const Buffer& buf)
        {
            _buffer = buf._buffer;
            _size = buf._size;
        }

        Buffer& operator=(const Buffer& buf)
        {
            _buffer = buf._buffer;
            _size = buf._size;
            return *this;
        }

        Buffer& operator=(char* buf)
        {
            _size = strlen(buf);
            _buffer = buf;
            return *this;
        }

        int size() const 
        {
            return _size;
        }

        void size(int size)
        {
            _size = size;
        }

        void set(char* buffer, int size)
        {
            _buffer = buffer;
            _size = size;
        }  

        char* operator()()
        {
            return _buffer;
        }
    private:
        char*   _buffer;
        int     _size;
};

}
}
#endif
