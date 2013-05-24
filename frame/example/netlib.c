#include "common.h"
D_CPREFIX
int package_complete(const char* buffer, int size)
{
    if(memchr(buffer, '\n', size)){
        return true;
    }
    return false;
}

D_CSURFIX
