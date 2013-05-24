#ifndef _DEVLIB_FRAME_CONFIG_H
#define _DEVLIB_FRAME_CONFIG_H
#include "common.h"
#include <libtcc.h>

//define structure & string at same time
#define TCC_DEFINE(tn, def)     \
    struct tn                    \
        def;                     \
    const static char tn ## _def_string[] = "struct " #tn #def ";";

TCC_DEFINE(frame_config, {
    char* listen_ip;
    char* run_dir;
    char* tcc_buffer;
    char* cblib;
    int threads;
    int nodelay;
    int recv_buffer_size;
    int send_buffer_size;
    int backlog;
    short listen_port;
})

int load_config(const char* filename, struct frame_config* config);
int init_config(struct frame_config* config);
void fini_config(struct frame_config* config);
#endif
