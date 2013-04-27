#ifndef _DEVLIB_FRAME_CONFIG_H
#define _DEVLIB_FRAME_CONFIG_H
#include "common.h"

#define CONF_DEFINE(tn, def)     \
    struct tn                    \
        def;                     \
    const static char tn ## _def_string[] = "struct " #tn #def ";";

/*
CONF_DEFINE(frame_config, {
    char listen_ip[16];
    char run_dir[256];
    short listen_port;
    int threads;
})*/
CONF_DEFINE(frame_config, {
    char* listen_ip;
    char* run_dir;
    short listen_port;
    int threads;
})

int load_config(const char* filename, struct frame_config* config);
#endif
