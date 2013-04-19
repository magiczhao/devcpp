#ifndef _DEVLIB_COMMON_H
#define _DEVLIB_COMMON_H
/**
 * commonly used constant value & macros & functions
 */
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#define offset_of(type, member)  ((int)(&((type*)0)->member))
#define container_of(ptr, type, member)     ((type*)(((char*)ptr) - offset_of(type, member)))

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define array_size(arr) (sizeof(arr) / sizeof(*(arr)))
#endif
