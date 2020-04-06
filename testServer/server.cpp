#include "server.h"

#include <iostream>
#include <functional>
#include <cstring>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

Server::Server() : Server(PORT)
{
}

Server::Server(int port) : port(port),
                           listenFd(createListenFd(port)),
                           listenRequest(new Request(listenFd)),
                           epoll(new Epoll()),
                           threadPool(new ThreadPool(WORKERS)),
                           timer(new TimerManager())
{
}
void Server::start()
{
    epoll->add(listenFd, listenRequest.get(), (EPOLLIN | EPOLLET));
    epoll->setOnConnection(std::bind(&Server::acceptConnection, this));
    epoll->setOnCloseConnection(std::bind(&Server::closeConnection, this, std::placeholders::_1));
    epoll->setOnRequest(std::bind(&Server::doRequest, this, std::placeholders::_1));
    epoll->setOnResponse(std::bind(&Server::doResponse, this, std::placeholders::_1));

    while (1)
    {
        int timeMS = timer->getNextExpireTimer();

        int eventsNum = epoll->wait(timeMS);
        // if (timeMS != 0)
        //     printf("hi had wait %dMS %d\n", timeMS, eventsNum);
        if (eventsNum > 0)
        {
            epoll->handleEvent(listenFd, threadPool, eventsNum);
        }

        timer->handleExpireTimers();
    }
}

void Server::acceptConnection()
{
    while (1)
    {

        int acceptFd = accept4(listenFd, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
        // printf("hi have connection fd = %d\n", acceptFd);
        if (acceptFd == -1)
        {
            if (errno == EAGAIN)
                break;

            printf("[Server::acceptConnection] accept : %s\n", strerror(errno));
            break;
        }

        Request *request = new Request(acceptFd);
        timer->addTimer(request, CONNECT_TIMEOUT, std::bind(&Server::closeConnection, this, request));
        epoll->add(acceptFd, request, (EPOLLIN | EPOLLONESHOT));
        int timeMS = timer->getNextExpireTimer();
        // printf("timeout = %dMS\n", timeMS);
    }
    // printf("I'am out\n");
}

void Server::closeConnection(Request *request)
{
    int fd = request->getFd();
    if (request->isWorking())
        return;

    timer->delTimer(request);

    epoll->del(fd, request, 0);
    delete request;
    request = nullptr;
}

void Server::doRequest(Request *request)
{
    timer->delTimer(request);
    int fd = request->getFd();

    int readErrno;
    int nRead = request->read(&readErrno);
    if (nRead == 0)
    {
        request->setNoWorking();
        closeConnection(request);
        return;
    }

    if (nRead < 0 && readErrno == EAGAIN)
    {
        epoll->mod(fd, request, (EPOLLIN | EPOLLONESHOT));
        request->setNoWorking();
        timer->addTimer(request, CONNECT_TIMEOUT, std::bind(&Server::closeConnection, this, request));
        return;
    }

    if (!request->parseRequest())
    {
        Response response(400, "", false);
        request->appendOutBuffer(response.makeResponse());

        int writeErrno;
        request->write(&writeErrno);
        request->setNoWorking();
        closeConnection(request);
        return;
    }
    request->getPath();
    request->keepAlive();
    if (request->parseFinsh())
    {

        Response response(200, request->getPath(), request->keepAlive());
        request->appendOutBuffer(response.makeResponse());
        epoll->mod(fd, request, (EPOLLIN | EPOLLOUT | EPOLLONESHOT));
    }
}

void Server::doResponse(Request *request)
{
    timer->delTimer(request);

    int fd = request->getFd();

    int toWrite = request->writeableBytes();

    if (toWrite == 0)
    {
        epoll->mod(fd, request, (EPOLLIN | EPOLLONESHOT));
        request->setNoWorking();
        timer->addTimer(request, CONNECT_TIMEOUT, std::bind(&Server::closeConnection, this, request));
        return;
    }

    int writeErrno;
    int ret = request->write(&writeErrno);

    if (ret < 0)
    {
        if (writeErrno == EAGAIN)
        {
            epoll->mod(fd, request, (EPOLLIN | EPOLLOUT | EPOLLONESHOT));
            request->setNoWorking();
            timer->addTimer(request, CONNECT_TIMEOUT, std::bind(&Server::closeConnection, this, request));
        }
        else
        {
            request->setNoWorking();
            closeConnection(request);
        }
        return;
    }

    if (ret == toWrite)
    {
        if (request->keepAlive())
        {
            request->resetParse();
            epoll->mod(fd, request, (EPOLLIN | EPOLLONESHOT));
            request->setNoWorking();
            timer->addTimer(request, CONNECT_TIMEOUT, std::bind(&Server::closeConnection, this, request));
        }
        else
        {
            request->setNoWorking();
            closeConnection(request);
        }

        return;
    }

    epoll->mod(fd, request, (EPOLLIN | EPOLLOUT | EPOLLONESHOT));
    request->setNoWorking();
    timer->addTimer(request, CONNECT_TIMEOUT, std::bind(&Server::closeConnection, this, request));
    return;
}