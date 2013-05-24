#ifndef DEVLIB_FRAME_DLOP_H
#define DEVLIB_FRAME_DLOP_H
#include <dlfcn.h>
struct connection;
typedef int(*handle_func)(int sock, struct connection* conn);
typedef int(*handle_buffer)(const char* buffer, int size);
struct frame_callback
{
    void *lib_handle;
    handle_func read_cb;
    handle_func write_cb;
    handle_func init_cb;
    handle_func fini_cb;
    handle_func timeout_cb;
};

struct net_util
{
    void* lib_handle;
    handle_buffer package_complete;
};

int load_callback(const char* lib, struct frame_callback*);
void fini_callback(struct frame_callback* cb);

int load_net_util(const char* lib, struct net_util*);
void fini_net_util(struct net_util*);

#endif
