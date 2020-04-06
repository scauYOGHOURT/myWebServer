#ifndef __SERVER_H__
#define __SERVER_H__

#include "request.h"
#include "response.h"
#include "utils.h"
#include "epoll.h"
#include "threadpool.h"
#include "timer.h"

#include <memory>
#include <mutex>

#define PORT 8888
#define TIMEOUT -1
#define CONNECT_TIMEOUT 500
#define WORKERS 4

class Server
{
public:
    Server();
    Server(int port);
    ~Server() {}
    void start(); //启动服务器

private:
    void acceptConnection();                //创建新连接
    void closeConnection(Request *request); //关闭连接
    void doRequest(Request *request);       //处理请求
    void doResponse(Request *request);      //处理响应

private:
    using ListenRequestPtr = std::unique_ptr<Request>;
    using EpollPtr = std::unique_ptr<Epoll>;
    using ThreadPoolPtr = std::shared_ptr<ThreadPool>;
    using TimerManagerPtr = std::unique_ptr<TimerManager>;

    int port;
    int listenFd;
    ListenRequestPtr listenRequest;
    EpollPtr epoll;
    ThreadPoolPtr threadPool;
    TimerManagerPtr timer;
};
#endif
