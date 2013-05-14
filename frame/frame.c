#include <event2/event.h>
#include "svr_config.h"
#include "svr_log.h"
#include "iohelper.h"
#include "common.h"
#include "dlock.h"
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>

int is_stop = false;
struct frame_config svrconf;
dlock_t thread_lock;
int init_svr_config(const char* file)
{
    if(init_config(&svrconf) != 0){
        return -1;
    }
    if(load_config(file, &svrconf) != 0){
        return -1;
    }
    return 0;
}

int create_listen_sock()
{
    int reuse_addr = 1;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1){
        error("create socket failed: %s", strerror(errno));
        goto fail;
    }
    if(set_nonblock(fd) != 0){
        error("set nonblock failed:%s", strerror(errno));
        goto fail;
    }
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) != 0){
        error("set reuseaddr failed:%s", strerror(errno));
        goto fail;
    }
    if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &svrconf.nodelay, sizeof(svrconf.nodelay)) != 0){
        error("set nodelay failed:%s", strerror(errno));
        goto fail;
    }
    if(setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &svrconf.send_buffer_size, sizeof(svrconf.send_buffer_size)) != 0){
        error("set send_buffer_size failed:%s", strerror(errno));
        goto fail;
    }
    if(setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &svrconf.recv_buffer_size, sizeof(svrconf.recv_buffer_size)) != 0){
        error("set recv_buffer_size failed:%s", strerror(errno));
        goto fail;
    }
    struct sockaddr_in svraddr;
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(svrconf.listen_port);
    svraddr.sin_addr.s_addr = inet_addr(svrconf.listen_ip);
    if(bind(fd, (struct sockaddr*)&svraddr, sizeof(svraddr)) != 0){
        error("bind addr[%s:%d] failed:%s", svrconf.listen_ip, svrconf.listen_port, strerror(errno));
        goto fail;
    }
    if(listen(fd, svrconf.backlog) != 0){
        error("listen failed:%s, backlog:%d", strerror(errno), svrconf.backlog);
        goto fail;
    }
    return 0;
fail:
    if(fd != -1){
        close(fd);
        fd = 0;
    }
    return -1;
}
void handle_sigint(int sig)
{
    is_stop = true;
}

void handle_child(int sig)
{
    pid_t child;
    int status;
    while((child = waitpid(-1, &status, WNOHANG)) > 0);
}

int setup_signal()
{
    //TODO setup signal functions
    if(SIG_ERR == signal(SIGINT, handle_sigint)){
        error("set SIGINT handler failed:%s", strerror(errno));
        return -1;
    }
    if(SIG_ERR == signal(SIGPIPE, SIG_IGN)){
        error("set SIGPIPE handler failed:%s", strerror(errno));
        return -1;
    }
    
    if(SIG_ERR == signal(SIGCHLD, handle_child)){
        error("set SIGCHLD handler failed:%s", strerror(errno));
        return -1;
    }
    info("setup signals ok!");
    return 0;
}

void* thread_loop(void* arg)
{
    struct event_base* base;
    base = event_base_new();
    if(!base){
        error("init event base failed!");
        return (void*) -1;
    }
    return NULL;
}

int main(int argc, char** argv)
{
    int is_daemon = 0;
    char conf_file[DPATH_MAX] = {0};
    char ch;
    pthread_t* workers = NULL;
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
    if(init_svr_config(conf_file) != 0){
        fprintf(stderr, "init config failed\n");
        return -1;
    }
    if(init_log() != 0){
        fprintf(stderr, "init log failed\n");
        return -1;
    }
    printf("config port:%d, config.ip:%s\n", svrconf.listen_port, svrconf.listen_ip);
    int sock = create_listen_sock();
    if(sock == -1){
        fprintf(stderr, "init socket failed\n");
        return -1;
    }
    info("socket listened!");
    if(dlock_init(&thread_lock) != 0){
        error("init lock failed:%s", strerror(errno));
        return -1;
    }
    if(svrconf.threads > 0){
        workers = (pthread_t*) malloc(sizeof(pthread_t) * svrconf.threads);
        for(int i = 0; i < svrconf.threads; ++i){
            if(pthread_create(workers + i, NULL, thread_loop, NULL) != 0){
                error("dispatch thread failed:%s", strerror(errno));
                is_stop = true;
                return -1;
            }
        }
    }else{
        //create only one thread
        workers = (pthread_t*) malloc(sizeof(pthread_t));
        if(pthread_create(workers, NULL, thread_loop, NULL) != 0){
            error("dispatch thread failed:%s", strerror(errno));
            is_stop = true;
            return -1;
        }
    }
    fini_config(&svrconf);
    return 0;
}
