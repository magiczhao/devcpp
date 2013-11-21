#ifndef DEVCPP_BIGMAP_TRACE_H
#define DEVCPP_BIGMAP_TRACE_H
#include <stdio.h>
namespace devcpp
{
namespace bigmap
{

#define DEBUG 1
#if defined(DEBUG)
#define trace(info, ...)     do{                 \
        printf("[TRACE]" info " [%s:%d]\n", ##__VA_ARGS__, __FILE__, __LINE__); \
    }while(0)

#else

#define trace(info, ...)

#endif
}
}
#endif
