#include "epoll.h"
#include "request.h"
#include "threadpool.h"

#include <iostream>
#include <cstring>

#include <unistd.h>

Epoll::Epoll() : epollFd(epoll_create1(EPOLL_CLOEXEC)), events(MAX_EVENT)
{
}

int Epoll::add(int fd, Request *request, int events)
{
    struct epoll_event event;
    event.data.ptr = (void *)request;
    event.events = events;
    int ret = epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event);
    return ret;
}

int Epoll::mod(int fd, Request *request, int events)
{
    struct epoll_event event;
    event.data.ptr = (void *)request;
    event.events = events;
    int ret = epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);

    return ret;
}

int Epoll::del(int fd, Request *request, int events)
{
    struct epoll_event event;
    event.data.ptr = (void *)request;
    event.events;
    int ret = epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &event);

    return ret;
}

int Epoll::wait(int timeoutMS)
{
    int eventsNum = epoll_wait(epollFd, &*events.begin(), static_cast<int>(events.size()), timeoutMS);

    if (eventsNum < 0)
        printf("[Epoll::wait] epoll : %s\n", strerror(errno));
    return eventsNum;
}

void Epoll::handleEvent(int listenFd, std::shared_ptr<ThreadPool> &threadpool, int eventsNum)
{
    for (int i = 0; i < eventsNum; i++)
    {
        Request *request = static_cast<Request *>(events[i].data.ptr);
        int fd = request->getFd();

        if (fd == listenFd)
        {
            onConnection();
        }
        else
        {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!events[i].events & EPOLLIN))
            {
                printf("sorry! have error\n");
                request->setNoWorking();
                onCloseConnection(request);
            }
            else if (events[i].events & EPOLLIN)
            {
                request->setWorking();
                threadpool->pushTask(std::bind(onRequest, request));
            }
            else if (events[i].events & EPOLLOUT)
            {
                request->setWorking();
                threadpool->pushTask(std::bind(onResponse, request));
            }
            else
            {
                printf("[Epoll::handleEvent] unexpected event\n");
            }
        }
    }

    return;
}

void Epoll::setOnConnection(const NewConnectionCallback &cb)
{
    onConnection = cb;
}

void Epoll::setOnCloseConnection(const CloseConnectionCallback &cb)
{
    onCloseConnection = cb;
}

void Epoll::setOnRequest(const HandleRequestCallback &cb)
{
    onRequest = cb;
}

void Epoll::setOnResponse(const HandleResponseCallback &cb)
{
    onResponse = cb;
}
