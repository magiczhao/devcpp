#ifndef _DEVLIB_FRAME_CONFIG_H
#define _DEVLIB_FRAME_CONFIG_H
#include "common.h"

//define structure & string at same time
#define TCC_DEFINE(tn, def)     \
    struct tn                    \
        def;                     \
    const static char tn ## _def_string[] = "struct " #tn #def ";";

TCC_DEFINE(frame_config, {
    char* listen_ip;
    char* run_dir;
    short listen_port;
    int threads;
})

int load_config(const char* filename, struct frame_config* config);
#endif
