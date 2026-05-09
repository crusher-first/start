#include "epoll_server.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

EpollServer::EpollServer(int port, int thread_num)
    : m_port(port), m_listenfd(-1), m_epollfd(-1), 
      m_thread_num(thread_num), m_pool(nullptr), m_timer(nullptr) {
    
    memset(m_fd_info, 0, sizeof(m_fd_info));
}

EpollServer::~EpollServer() {
    Close();
}

int EpollServer::SetNonBlocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

void EpollServer::Addfd(int epollfd, int fd, bool oneshot) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    if (oneshot) {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    SetNonBlocking(fd);
}

void EpollServer::AddTimer(int fd, HttpRequest* request) {
    if (m_timer) {
        m_timer->AddTimer(fd, 30, [this, fd]() {
            HandleClose(fd);
        });
    }
}

void EpollServer::HandleNewConnection() {
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    int connfd = accept(m_listenfd, (struct sockaddr*)&client_addr, &client_addrlen);
    
    if (connfd < 0) {
        printf("accept error: %s\n", strerror(errno));
        return;
    }

    if (connfd >= MAX_FD) {
        close(connfd);
        return;
    }

    Addfd(m_epollfd, connfd, true);

    m_fd_info[connfd] = new FdInfo();
    m_fd_info[connfd]->request = new HttpRequest();
    m_fd_info[connfd]->read_buffer = new Buffer();
    m_fd_info[connfd]->write_buffer = new Buffer();
    m_fd_info[connfd]->m_sockfd = connfd;

    AddTimer(connfd, m_fd_info[connfd]->request);
}

void EpollServer::HandleRead(int fd) {
    m_timer->DelTimer(fd);
    
    Buffer* read_buf = m_fd_info[fd]->read_buffer;
    char buffer[4096];
    
    memset(buffer, 0, sizeof(buffer));
    ssize_t ret = recv(fd, buffer, sizeof(buffer), 0);
    
    if (ret <= 0) {
        HandleClose(fd);
        return;
    }

    read_buf->Append(buffer, ret);

    HttpRequest* request = m_fd_info[fd]->request;
    HttpRequest::HTTP_CODE code = request->Parse(
        const_cast<char*>(read_buf->GetReadPtr()), 
        read_buf->GetReadableBytes());

    if (code == HttpRequest::NO_REQUEST) {
        return;
    }

    // 生成响应
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body>Hello World</body></html>";
    m_fd_info[fd]->write_buffer->Append(response.c_str(), response.size());
    
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
    epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &event);
}

void EpollServer::HandleWrite(int fd) {
    Buffer* write_buf = m_fd_info[fd]->write_buffer;
    const char* data = write_buf->GetReadPtr();
    int len = write_buf->GetReadableBytes();

    ssize_t ret = send(fd, data, len, 0);

    if (ret > 0) {
        write_buf->Retrieve(ret);
        
        if (write_buf->GetReadableBytes() > 0) {
            struct epoll_event event;
            event.data.fd = fd;
            event.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
            epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &event);
        } else {
            HttpRequest* request = m_fd_info[fd]->request;
            if (request->IsKeepAlive()) {
                request->Reset();
                struct epoll_event event;
                event.data.fd = fd;
                event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &event);
                AddTimer(fd, request);
            } else {
                HandleClose(fd);
            }
        }
    } else {
        HandleClose(fd);
    }
}

void EpollServer::HandleClose(int fd) {
    if (m_fd_info[fd]) {
        delete m_fd_info[fd]->request;
        delete m_fd_info[fd]->read_buffer;
        delete m_fd_info[fd]->write_buffer;
        delete m_fd_info[fd];
        m_fd_info[fd] = nullptr;
    }
    epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
}

void EpollServer::Start() {
    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenfd < 0) {
        perror("socket");
        exit(1);
    }

    int flag = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(m_port);

    if (bind(m_listenfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    listen(m_listenfd, 5);

    m_epollfd = epoll_create(5);
    Addfd(m_epollfd, m_listenfd, false);

    m_pool = new ThreadPool(m_thread_num);
    m_timer = new Timer();

    epoll_event events[MAX_EVENTS];
    printf("Server started on port %d\n", m_port);

    while (true) {
        m_timer->Tick();
        
        int timeout = m_timer->GetNextTick();
        int nfds = epoll_wait(m_epollfd, events, MAX_EVENTS, timeout > 0 ? timeout * 1000 : 1000);

        for (int i = 0; i < nfds; ++i) {
            int fd = events[i].data.fd;
            
            if (fd == m_listenfd) {
                HandleNewConnection();
            } else if (events[i].events & (EPOLLIN | EPOLLERR)) {
                m_pool->AddTask([this, fd]() {
                    HandleRead(fd);
                });
            } else if (events[i].events & EPOLLOUT) {
                m_pool->AddTask([this, fd]() {
                    HandleWrite(fd);
                });
            }
        }
    }
}

void EpollServer::Close() {
    if (m_listenfd >= 0) close(m_listenfd);
    if (m_epollfd >= 0) close(m_epollfd);
    if (m_pool) delete m_pool;
    if (m_timer) delete m_timer;
    
    for (int i = 0; i < MAX_FD; ++i) {
        if (m_fd_info[i]) {
            HandleClose(i);
        }
    }
}
