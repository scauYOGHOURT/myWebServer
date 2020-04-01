#ifndef __SERVER_H__
#define __SERVER_H__

#define PORT 8888
#define TIMEOUT -1
#define CONNECT_TIMEOUT 500
#define WORKERS 4

#include <memory>
#include <mutex>

class ThreadPool;
class Request;
class MyEpoll;
class TimerManager;

class Server
{
public:
    Server() {}
    Server(int port) {}
    ~Server() {}
    void start() {} //启动服务器

private:
    void acceptConnect() {}                //创建新连接
    void closeConnect(Request *request) {} //关闭连接
    void doRequest(Request *request) {}    //处理请求
    void doResponse(Request *request) {}   //处理响应

private:
    using ListenRequestPtr = std::unique_ptr<Request>;
    using EpollPtr = std::unique_ptr<MyEpoll>;
    using ThreadPoolPtr = std::shared_ptr<ThreadPool>;
    using TimerManagerPtr = std::unique_ptr<TimerManager>;

    int port;
    int listenfd;
    ListenRequestPtr listenRequest;
    EpollPtr epoll;
    ThreadPoolPtr threadPool;
    TimerManagerPtr timer;
};
#endif
