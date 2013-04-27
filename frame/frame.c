#include <event2/event.h>
#include "svr_config.h"

struct frame_config svrconf;

int main(int argc, char** argv)
{
    load_config("./text.conf", &svrconf);
    printf("config port:%d, config.ip:%s\n", svrconf.listen_port, svrconf.listen_ip);
    return 0;
}
