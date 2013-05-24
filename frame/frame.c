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
    return fd;
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

void thread_timer(evutil_socket_t sock, short ev_flag, void* arg)
{
    //info("thread working!");
    //TODO add timing here
}

int handle_read(int sock, struct connection* conn)
{
    int ret = recv(sock, buffer_position(&(conn->readbuf)), buffer_space(&(conn->readbuf)), 0);
    switch(ret){
        case 0://connection closed
            info("connection closed by peer");
            return -1;
        case -1://error
            if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR){
                return 0;
            }else{
                error("recv failed:%s", strerror(errno));
                return -1;
            }
        default:
            buffer_write(&(conn->readbuf), ret);
    }
    //test if package complete, if ok, process the package
    return 0;
}

int handle_write(int sock, struct connection* conn)
{
    int ret = send(sock, buffer_position(&(conn->writebuf)), buffer_space(&(conn->writebuf)), 0);
    switch(ret){
        case 0://connection closed
            info("connection closed by peer in write");
            return -1;
        case -1:
            if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR){
                return 0;
            }else{
                error("send failed:%s", strerror(errno));
                return -1;
            }
        default:
            buffer_write(&(conn->writebuf), ret);
    }
    //test if package complete, if ok, process the package
    return 0;
}

int handle_init(int sock, struct connection* conn)
{
    return 0;
}

int handle_timeout(int sock, struct connection* conn)
{
    return -1;
}

void thread_client(evutil_socket_t sock, short ev_flag, void* arg)
{
    info("aaa :%d", ev_flag);
    int handle_result = 0;
    int fd = -1;
    struct connection* conn = (struct connection*)arg;
    switch(ev_flag){
        case EV_READ:
            handle_result = handle_read(sock, conn);
            break;
        case EV_WRITE:
            handle_result = handle_write(sock, conn);
            break;
        case EV_TIMEOUT:
            handle_result = handle_timeout(sock, conn);
            break;
        default:
            handle_result = -1;
            error("unknown ev_flag occurs:%d", ev_flag);
    }
    switch(handle_result){
        case 0://OK, then continue next
        case 1://not complete
            break;
        case -1://failed
            info("free connection!");
            fd = event_get_fd(conn->evt);
            if(fd >= 0){
                close(fd);
            }
            event_free(conn->evt);
            conn->evt = NULL;
            free_connection(conn);
            break;
        default:
            break;
            //unknown
    }
    return;
}

void thread_accept(evutil_socket_t sock, short ev_flag, void* arg)
{
    struct sockaddr_in addr;
    struct event_pool* ep = (struct event_pool*)arg;
    socklen_t addrlen = sizeof(addr);
    int fd = accept(sock, (struct sockaddr*)&addr, &addrlen);
    if(fd == -1){
        error("accept client faile:%s", strerror(errno));
        return;
    }
    struct connection* conn = get_connection(ep);
    if(!conn){
        error("not enough memory for handle this connection");
        close(fd);
        return;
    }
    struct event* evt_client = event_new(ep->base, fd, EV_READ, thread_client, conn);
    conn->evt = evt_client;
    if(handle_init(fd, conn) == 0){
        event_add(evt_client, NULL);
        event_base_loopbreak(ep->base);
    }else{
        error("init connection from client failed");
        close(fd);
        free_connection(conn);
        return;
    }
}

void* thread_loop(void* arg)
{
    int svrsock = (int) arg;
    //struct event_base* base;
    struct event* evt_timer = NULL;
    struct event* evt_listen = NULL;
    int ret = -1;
    struct event_pool ep;
    if(fixed_pool_init(&ep.mempool, 1000, 1000) != 0){
        error("init memory pool failed!");
        goto err;
    }
    ep.is_leader = false;
    ep.cfg = event_config_new();
    if(!ep.cfg){
        error("init event config failed!");
        goto err;
    }else{
        event_config_require_features(ep.cfg, 0);
    }
    ep.base = event_base_new_with_config(ep.cfg);
    if(!ep.base){
        error("init event base failed!");
        goto err;
    }else{
        int feature = event_base_get_features(ep.base);
        if(feature & EV_FEATURE_ET) info("edge triger used");
        if(feature & EV_FEATURE_O1) info("feature 01 used");
        if(feature & EV_FEATURE_FDS) info("feature fds used");
    }
    evt_timer = event_new(ep.base, -1, EV_PERSIST, thread_timer, NULL);
    evt_listen = event_new(ep.base, svrsock, EV_READ | EV_PERSIST, thread_accept, &ep);
    if((!evt_timer) || (!evt_listen)){
        error("init svr sock or timer failed!");
        goto err;
    }
    
    struct timeval interval = {1, 0};
    event_add(evt_timer, &interval);
    while(!is_stop){
        if(dlock_trylock(&thread_lock) == 0){
            event_add(evt_listen, NULL);
            ep.is_leader = true;
        }else{
            ep.is_leader = false;
        }
        int ret = event_base_loop(ep.base, 0);
        switch(ret){
            case -1:
                error("event base dispatch loop failed!");
                break;
            case 1:
                info("no events!");
                break;
            default:
                break;
        }
        if(ep.is_leader){
            dlock_unlock(&thread_lock);
            event_del(evt_listen);
            ep.is_leader = false;
        }
    }
    ret = 0;
err:
    event_free(evt_timer);
    event_free(evt_listen);
    evt_listen = NULL;
    evt_timer = NULL;
    event_pool_fini(&ep);
    return (void*) ret;
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
    info("socket listened, fd:%d!", sock);
    if(dlock_init(&thread_lock) != 0){
        error("init lock failed:%s", strerror(errno));
        return -1;
    }
    info("init threads, total:%d thread", svrconf.threads);
    if(svrconf.threads > 0){
        workers = (pthread_t*) malloc(sizeof(pthread_t) * svrconf.threads);
        for(int i = 0; i < svrconf.threads; ++i){
            if(pthread_create(workers + i, NULL, thread_loop, (void*)sock) != 0){
                error("dispatch thread failed:%s", strerror(errno));
                is_stop = true;
                return -1;
            }
        }
    }else{
        //create only one thread
        workers = (pthread_t*) malloc(sizeof(pthread_t));
        if(pthread_create(workers, NULL, thread_loop, (void*)sock) != 0){
            error("dispatch thread failed:%s", strerror(errno));
            is_stop = true;
            return -1;
        }
    }
    info("thread init ok!");
    int thread_count = svrconf.threads > 0 ? svrconf.threads : 1;
    //waiting all threads to finish
    for(int i = 0; i < thread_count; ++i){
        void *tresult;
        if(pthread_join(workers[i], &tresult) != 0){
            error("join thread:%d failed:%s", (pthread_t)workers[i], strerror(errno));
        }
    }
    free(workers);
    workers = NULL;
    info("all thread exit safely!");
    fini_log();
    fini_config(&svrconf);
    return 0;
}
