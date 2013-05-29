#ifndef DEVLIB_FRAME_DLOP_H
#define DEVLIB_FRAME_DLOP_H
#include <dlfcn.h>
struct connection;
/** client event callback function */
typedef int(*handle_func)(int sock, struct connection* conn);
/** network package process callback function */
typedef int(*handle_buffer)(const char* buffer, int size);

struct frame_callback
{
    /** dynamic library handle */
    void *lib_handle;
    /** callback functions here */
    handle_func read_cb; //calls on read event
    handle_func write_cb; //calls on write event
    handle_func init_cb; //calls on client connect
    handle_func fini_cb; //calls on client close
    handle_func timeout_cb; //calls on timeout
};

struct net_util
{
    /** dynamic library handle */
    void* lib_handle;
    /** call on recv, check if the package finish */
    handle_buffer package_complete;
};

int load_callback(const char* lib, struct frame_callback*);
void fini_callback(struct frame_callback* cb);

int load_net_util(const char* lib, struct net_util*);
void fini_net_util(struct net_util*);

#endif
