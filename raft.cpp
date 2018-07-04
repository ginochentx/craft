#include <stdio.h>
#include <stdint.h>
#include <string>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <assert.h>
#include <pthread.h>

using namespace std;
enum NodeRole{
    FOLLOWER = 0,
    CANDIDATE,
    MASTER,
};

class Node {
public:
    Node();

public:
    NodeRole role_; // 0 -> follower; 1 -> candidate; 2 -> master
    uint32_t current_term_;
    uint32_t vote_for_;
    uint32_t node_id_;
    //vector<string> log_;

    //uint32_t commit_index_;
    //uint32_t last_applied_;

    //vector<uint32_t> next_index_;

};

Node::Node() {
    role_ = FOLLOWER;
    current_term_ = 0;
    vote_for_ = 0xffffffff;

    //commit_index_ = 0;
    //last_applied_ = 0;
}

string g_ip;
uint32_t g_port;
Node g_node;
time_t g_last_heartbeat;
uint32_t g_timeout;
uint32_t g_gap;

static int socket_bind(const char* ip,int port)
{
    int  listenfd;
    struct sockaddr_in servaddr;
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    if (listenfd == -1)
    {
        perror("socket error:");
        exit(1);
    }
    memset(&servaddr, 0x0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&servaddr.sin_addr);
    servaddr.sin_port = htons(port);
    if (bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) == -1)
    {
        perror("bind error: ");
        exit(1);
    }
    return listenfd;
}

#define MAXSIZE     1024
#define FDSIZE      1000
#define EPOLLEVENTS 100

static void add_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
}

static void delete_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);
}

static void modify_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);
}

static void handle_accpet(int epollfd,int listenfd)
{
    int clifd;
    struct sockaddr_in cliaddr;
    socklen_t  cliaddrlen;
    clifd = accept(listenfd,(struct sockaddr*)&cliaddr,&cliaddrlen);
    if (clifd == -1)
        perror("accpet error:");
    else
    {
        printf("accept a new client: %s:%d\n",inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port);
        add_event(epollfd,clifd,EPOLLIN|EPOLLOUT);
    }
}

struct RequestVote {
    uint32_t term;
    uint32_t candidate_id;
    //uint32_t last_log_index;
    //uint32_t last_log_term;
};

struct RequestVoteRes {
    uint32_t term_;
    uint32_t vote_granted_; //1: means candidate recevied voted, else 0.
};

static void do_request_vote(const RequestVote* requestVote) {
    return;
}

static void do_request_vote_res(const RequestVoteRes* requestVoteRes) {
    return;
}

static void do_read(int epollfd,int fd)
{
    char buf[MAXSIZE];
    int nread;
    nread = read(fd,buf,MAXSIZE);
    if (nread == -1)
    {
        perror("read error:");
        goto err_t;
    }
    else if (nread == 0)
    {
        fprintf(stderr,"client close.\n");
        goto err_t;
    }
    else if (buf[0] == 'r') {
        assert(nread == sizeof(RequestVote));
        do_request_vote((RequestVote*)(buf+1));
    }
    else if (buf[0] == 'R') {
        assert(nread == sizeof(RequestVoteRes));
        do_request_vote_res((RequestVoteRes*)(buf+1));
    }
    else {
        perror("unkown package:");
        goto err_t;
    }

    return;

err_t:
    close(fd);
    delete_event(epollfd,fd,EPOLLIN);
}

static void do_write(int epollfd,int fd,char *buf)
{
    int nwrite;
    nwrite = write(fd,buf,strlen(buf));
    if (nwrite == -1)
    {
        perror("write error:");
        close(fd);
        delete_event(epollfd,fd,EPOLLOUT);
    }
    else
    {
    }
}

static void handle_events(int epollfd,struct epoll_event *events,int num,int listenfd)
{
    int i;
    int fd;
    for (i = 0;i < num;i++)
    {
        fd = events[i].data.fd;
        if ((fd == listenfd) &&(events[i].events & EPOLLIN))
            handle_accpet(epollfd,listenfd);
        else if (events[i].events & EPOLLIN)
            do_read(epollfd,fd);
        else if (events[i].events & EPOLLOUT){
            //do_write(epollfd,fd);
            //TODO
        }
    }
}

static void start_vote() {
    RequestVote requestVote;

    //requestVote.term_= ++g_node.current_term_;
    //requestVote.candidate_id = g_node.node_id_;

    //for (uint32_t i = 0; i < g_nodes.size(); i++) {

    //}

    return;
}

static void do_epoll(int listenfd)
{
    int epollfd;
    struct epoll_event events[EPOLLEVENTS];
    int ret;
    epollfd = epoll_create(FDSIZE);
    add_event(epollfd,listenfd,EPOLLIN);
    for (;;)
    {
        ret = epoll_wait(epollfd,events,EPOLLEVENTS,g_timeout);
        if (-1 == ret) {
            perror("epoll_wait error: ");
            goto err_t;
        } else if (0 == ret) {
            if (time(NULL) - g_last_heartbeat > g_gap) {
                g_node.role_ = CANDIDATE;
                start_vote();
            }
        }

        handle_events(epollfd,events,ret,listenfd);
    }
    close(epollfd);

err_t:
    close(epollfd);
    exit(1);
}

void* start_raft(void *unused) {
    int ret = 0;

    int  listenfd;
    listenfd = socket_bind(g_ip.c_str(),g_port);
    //listen(listenfd,5);
    //if (ret != 0) {
    //    perror("listen error: ");
    //    exit(1);
    //}

    do_epoll(listenfd);
    
    return 0;
}

void init_raft() {
    int ret = 0;
    pthread_t tid;
    ret = pthread_create(&tid, NULL, start_raft, NULL);
    if (ret != 0) {
        perror("pthread create failed:");
        abort();
    }

    return;
}
