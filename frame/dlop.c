#include "dlop.h"
#include "svr_log.h"
#include "common.h"

#define install_cb(hdl, name, ptr) do{  \
        ptr = dlsym(hdl, name);         \
        if(!ptr){                       \
            goto err;                   \
        }                               \
    }while(0)


void fini_callback(struct frame_callback* cb)
{
    if(cb->lib_handle){
        dlclose(cb->lib_handle);
        memset(cb, 0, sizeof(struct frame_callback));
    }
}

int load_callback(const char* lib, struct frame_callback* cb)
{
    memset(cb, 0, sizeof(struct frame_callback));

    int ret = -1;
    void* handle = dlopen(lib, RTLD_NOW);
    if(!handle){
        error("open dynamic library failed:%s", dlerror());
        goto err;
    }
    cb->lib_handle = handle;
    install_cb(handle, "handle_read", cb->read_cb);
    install_cb(handle, "handle_write", cb->write_cb);
    install_cb(handle, "handle_init", cb->init_cb);
    install_cb(handle, "handle_fini", cb->fini_cb);
    install_cb(handle, "handle_timeout", cb->timeout_cb);
    ret = 0;
err:
    if(handle && ret == -1){
        dlclose(handle);
        handle = NULL;
    }
    return ret;
}
int load_net_util(const char* lib, struct net_util* util)
{
    int ret = -1;
    memset(util, 0, sizeof(struct net_util));
    void* handle = dlopen(lib, RTLD_NOW);
    if(!handle){
        error("open dynamic library failed:%s", dlerror());
        goto err;
    }
    util->lib_handle = handle;
    install_cb(handle, "package_complete", util->package_complete);
    ret = 0;
err:
    if(handle && ret == -1){
        dlclose(handle);
        handle = NULL;
    }
    return ret;
}

void fini_net_util(struct net_util* util)
{ 
    if(util->lib_handle){
        dlclose(util->lib_handle);
        memset(util, 0, sizeof(struct net_util));
    }
}
#undef install_cb
