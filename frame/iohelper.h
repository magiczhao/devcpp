#ifndef DEVLIB_FRAME_IOHELPER_H
#define DEVLIB_FRAME_IOHELPER_H
#include "common.h"
#include "event2/event.h"
#include "dstring.h"
#include "fixed_alloc.h"
struct dbuffer
{
    char* buffer;
    int capacity;
    int size;
};
#define buffer_position(buf) ((buf)->buffer + (buf)->size)
#define buffer_space(buf) ((buf)->capacity - (buf)->size)
#define buffer_write(buf, val) do{(buf)->size += val;}while(0)

struct connection
{
    struct event* evt;
    struct dbuffer readbuf;
    struct dbuffer writebuf;
};

struct event_pool
{
    struct fixed_dmem_pool mempool;
    struct event_base* base;
    int is_leader;
};

struct connection* get_connection(struct event_pool* ep);
void event_pool_fini(struct event_pool*);
int set_nonblock(int fd);
#endif
