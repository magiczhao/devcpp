#include "common.h"
#include "iohelper.h"

int handle_read(int sock, struct connection* conn)
{
    memcpy(conn->writebuf.buffer, conn->readbuf.buffer, conn->readbuf.size);
    conn->writebuf.size = 0;
    conn->writebuf.capacity = conn->readbuf.size;
    buffer_clear((&conn->readbuf));
    enable_write(conn);
    return 0;
}

int handle_write(int sock, struct connection* conn)
{
    if(buffer_space(&(conn->writebuf)) == 0){
        disable_write(conn);
    }
    return 0;
}

int handle_timeout(int sock, struct connection* conn)
{
    return -1;
}

int handle_init(int sock, struct connection* conn)
{
    conn->timeout.tv_sec = 5;
    return 0;
}

int handle_fini(int sock, struct connection* conn)
{
    return 0;
}
