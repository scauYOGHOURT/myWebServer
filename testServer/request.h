#ifndef __REQUEST_H__
#define __REQUEST_H__

#include "buffer.h"

#include <string>
#include <map>
#include <iostream>

class Timer;
class Request
{
public:
    enum HttpRequestParseState
    {
        ExpectRequestLine,
        ExpectHeaders,
        ExpectBody,
        GotAll
    };

    enum Method
    {
        Invalid,
        Get,
        Post,
        Head,
        Put,
        Delete
    };

    enum Version
    {
        Unknown,
        HTTP10,
        HTTP11
    };

    Request(int fd);

    int getFd();               //返回文件描述符
    int read(int *savedErrno); //读数据
    int write(int *saveErrno); //写数据

    void appendOutBuffer(const Buffer &buf);
    size_t writeableBytes();

    void setTimer(Timer *timer);
    Timer *getTimer();

    void setWorking();
    void setNoWorking();
    bool isWorking() const;

    bool parseRequest(); //解析HTTP报文
    bool parseFinsh();   //报文解析完成
    void resetParse();   //重置报文状态
    std::string getPath() const;
    std::string getQuery() const;
    std::string getHeader(const std::string &field) const;
    std::string getMethod() const;
    bool keepAlive() const;

    ~Request();

private:
    bool parseRequestLine(const char *begin, const char *end);             //解析请求行
    bool setMethod(const char *begin, const char *end);                    //设置HTTP方法
    void setPath(const char *begin, const char *end);                      //设置URL路径
    void setQuery(const char *begin, const char *end);                     //设置URL参数
    void setVersion(Version version);                                      //设置HTTP版本
    void addHeader(const char *start, const char *colon, const char *end); //增加报文头部

private:
    int fd;
    Buffer inBuff;
    Buffer outBuff;
    bool working;

    Timer *timer;

    HttpRequestParseState state;
    Method method;
    Version version;
    std::string path;
    std::string query;
    std::map<std::string, std::string> headers;
};
#endif