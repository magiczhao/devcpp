#ifndef _DEVLIB_COMMON_H
#define _DEVLIB_COMMON_H

#define offset_of(type, member)  ((int)(&((type*)0)->member))
#define container_of(ptr, type, member)     ((type*)(((char*)ptr) - offset_of(type, member)))

#endif
