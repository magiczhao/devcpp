#include "iohelper.h"

inline int set_nonblock(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    if(fcntl(fd, F_SETFL, flag | O_NONBLOCK) != 0) return -1;
    return 0;
}

void event_pool_fini(struct event_pool* ep)
{
    if(ep->base){
        event_base_free(ep->base);
        ep->base = NULL;
    }
    fixed_pool_fini(&ep->mempool);
}

struct connection* get_connection(struct event_pool* ep)
{
    const static int read_size = 384;
    const static int write_size = 384;
    struct connection* conn = (struct connection*) d_fp_malloc(&ep->mempool, read_size + write_size + sizeof(struct connection));
    if(conn){
        //TODO init readbuf & writebuf here
        char* p = (char*) conn;
        conn->readbuf.buffer = p + sizeof(struct connection);
        conn->readbuf.capacity = read_size;
        conn->readbuf.size = 0;
        conn->writebuf.buffer = p + sizeof(struct connection) + read_size;
        conn->writebuf.capacity = write_size;
        conn->writebuf.size = 0;
    }
    return conn;
}
