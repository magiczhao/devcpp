#ifndef DEVLIB_FRAME_IOHELPER_H
#define DEVLIB_FRAME_IOHELPER_H
#include "common.h"
#include "event2/event.h"
#include "dstring.h"
#include "fixed_alloc.h"
struct dbuffer
{
    int capacity;
    int size;
    char* buffer;
};
#define buffer_position(buf) ((buf)->buffer + (buf)->size)
#define buffer_space(buf) ((buf)->capacity - (buf)->size)
#define buffer_write(buf, val) do{(buf)->size += val;}while(0)
struct event_pool
{
    struct fixed_dmem_pool mempool;
    struct event_config* cfg;
    struct event_base* base;
    int is_leader;
};

struct connection
{
    struct event* evt;
    struct event_pool* ep;
    struct dbuffer readbuf;
    struct dbuffer writebuf;
};

struct connection* get_connection(struct event_pool* ep);
void free_connection(struct connection* conn);
void event_pool_fini(struct event_pool*);
int set_nonblock(int fd);
#endif
