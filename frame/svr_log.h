#ifndef DEVLIB_FRAME_SVR_LOG_H
#define DEVLIB_FRAME_SVR_LOG_H
#include <log4c.h>

extern log4c_category_t* log_root_cat;
extern log4c_category_t* log_simple_cat;
extern int _log4c_inited;
#define __log(level, fmt, ...)    do{             \
        if(_log4c_inited){                      \
            log4c_category_log(log_root_cat, level, fmt "[%s:%d]", ##__VA_ARGS__, __FILE__, __LINE__);  \
        }                                       \
    }while(0)

#define debug(fmt, ...)  __log(LOG4C_PRIORITY_DEBUG, fmt, ##__VA_ARGS__)
#define info(fmt, ...)  __log(LOG4C_PRIORITY_INFO, fmt, ##__VA_ARGS__)
#define error(fmt, ...)  __log(LOG4C_PRIORITY_ERROR, fmt, ##__VA_ARGS__)
int init_log();
void fini_log();
#endif
