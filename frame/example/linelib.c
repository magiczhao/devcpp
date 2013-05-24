#include "common.h"

int handle_read(int sock, struct connection* conn)
{
    memcpy(conn->writebuf.buffer, conn->readbuf.buffer, conn->readbuf.size);
    buffer_write(&conn->writebuf, conn->readbuf.size);
    enable_write(conn);
    return 0;
}

int handle_write(int sock, struct connection* conn)
{
    return 0;
}

int handle_timeout(int sock, struct connection* conn)
{
    return -1;
}

int handle_init(int sock, struct connection* conn)
{
    return 0;
}

int handle_fini(int sock, struct connection* conn)
{
    return 0;
}
