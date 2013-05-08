#include "iohelper.h"

inline int set_nonblock(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    if(flag != 0) return -1;
    if(fcntl(fd, F_SETFL, flag | O_NONBLOCK) != 0) return -1;
    return 0;
}
