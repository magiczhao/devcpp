#ifndef _DEVLIB_CSTRING_H
#define _DEVLIB_CSTRING_H
struct dstring
{
    int size;
    char* buffer;
};

#define dstring_assign(str, buf, sz)  do{   \
            (str)->size = (sz);             \
            (str)->buffer = buf;            \
        }while(0)

#define dstring_size(str)       ((str)->size)
#define dstring_buffer(str)     ((str)->buffer)
#endif
