#include "svr_log.h"

log4c_category_t* log_root_cat = NULL;
log4c_category_t* log_simple_cat = NULL;
int _log4c_inited = 0;

int init_log()
{
    log4c_init();
    log_root_cat = log4c_category_get("root");
    log_simple_cat = log4c_category_get("simple");
    if(log_root_cat && log_simple_cat){
        _log4c_inited = 1;
        return 0;
    }
    return -1;
}

void fini_log()
{
    if(_log4c_inited){
        log_root_cat = NULL;
        log_simple_cat = NULL;
        log4c_fini();
        _log4c_inited = 0;
    }
}
