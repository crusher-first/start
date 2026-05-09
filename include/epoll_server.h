#ifndef EPOLL_SERVER_H
#define EPOLL_SERVER_H

#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <errno.h>
#include "threadpool.h"
#include "timer.h"
#include "http.h"
#include "buffer.h"

class EpollServer {
public:
    EpollServer(int port = 8080, int thread_num = 4);
    ~EpollServer();

    void Start();
    void Close();

private:
    int SetNonBlocking(int fd);
    void Addfd(int epollfd, int fd, bool oneshot = false);
    void AddTimer(int fd, HttpRequest* request);
    void HandleNewConnection();
    void HandleRead(int fd);
    void HandleWrite(int fd);
    void HandleClose(int fd);

    static const int MAX_EVENTS = 1024;
    static const int MAX_FD = 65536;

    int m_port;
    int m_listenfd;
    int m_epollfd;
    int m_thread_num;

    ThreadPool* m_pool;
    Timer* m_timer;

    struct FdInfo {
        HttpRequest* request;
        Buffer* read_buffer;
        Buffer* write_buffer;
        int m_sockfd;
    };

    FdInfo* m_fd_info[MAX_FD];
};

#endif // EPOLL_SERVER_H
