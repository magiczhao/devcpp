#ifndef _DEVLIB_COMMON_H
#define _DEVLIB_COMMON_H
/**
 * commonly used constant value & macros & functions
 */
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#define offset_of(type, member)  ((int)(&((type*)0)->member))
#define container_of(ptr, type, member)     ((type*)(((char*)ptr) - offset_of(type, member)))

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifdef _POSIX_PATH_MAX
#define DPATH_MAX  _POSIX_PATH_MAX
#else
#define DPATH_MAX 256
#endif

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>

#define USE_LIKELY
#ifdef USE_LIKELY
#define likely(x) __buildin_expect(!!(x), 1)
#define unlikely(x) __buildin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#define array_size(arr) (sizeof(arr) / sizeof(*(arr)))

#ifdef __cplusplus
#define D_CPREFIX   extern "C"{
#define D_CSURFIX   }
#else
#define D_CPREFIX 
#define D_CSURFIX
#endif

#endif
