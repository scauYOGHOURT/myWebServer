#ifndef __EPOLL_H__
#define __EPOLL_H__

#include <functional>
#include <vector>
#include <memory>

#include <sys/epoll.h>

#define MAX_EVENT 1024

class Request;
class ThreadPool;

class Epoll
{
public:
    using NewConnectionCallback = std::function<void()>;
    using CloseConnectionCallback = std::function<void(Request *)>;
    using HandleRequestCallback = std::function<void(Request *)>;
    using HandleResponseCallback = std::function<void(Request *)>;

    Epoll();
    int add(int fd, Request *request, int event);
    int mod(int fd, Request *request, int event);
    int del(int fd, Request *request, int event);
    int wait(int timeoutMs);

    void handleEvent(int listenFd, std::shared_ptr<ThreadPool> &threadPool, int eventsNum);
    void setOnConnection(const NewConnectionCallback &cb);
    void setOnCloseConnection(const CloseConnectionCallback &cb);
    void setOnRequest(const HandleRequestCallback &cb);
    void setOnResponse(const HandleResponseCallback &cb);

private:
    int epollFd;
    std::vector<struct epoll_event> events;
    NewConnectionCallback onConnection;
    CloseConnectionCallback onCloseConnection;
    HandleRequestCallback onRequest;
    HandleResponseCallback onResponse;
};

#endif