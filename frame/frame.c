#include <event2/event.h>
#include "svr_config.h"
#include "common.h"
#include <unistd.h>
#include <getopt.h>

struct frame_config svrconf;

int main(int argc, char** argv)
{
    int is_daemon = 0;
    char conf_file[DPATH_MAX] = {0};
    char ch;
    while((ch = getopt(argc, argv, "c:d")) != -1){
        switch(ch){
            case 'c':
                strncpy(conf_file, optarg, sizeof(conf_file));
                break;
            case 'd':
                is_daemon = 1;
                break;
            default:
                fprintf(stderr, "option %c is not defined\n", ch);
        }
    }
    (void)(sizeof(is_daemon));
    if(strlen(conf_file) == 0){
        fprintf(stderr, "not set config file\n");
        return -1;
    }
    load_config(conf_file, &svrconf);
    printf("config port:%d, config.ip:%s\n", svrconf.listen_port, svrconf.listen_ip);
    return 0;
}
