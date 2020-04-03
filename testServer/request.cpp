#include "request.h"
#include "timer.h"

#include <iostream>
#include <cassert>

#include <unistd.h>

Request::Request(int fd) : fd(fd),
                           working(false),
                           timer(nullptr),
                           state(ExpectRequestLine),
                           method(Invalid),
                           version(Unknown)
{
}

int Request::getFd()
{
    return fd;
}

int Request::read(int *savedErrno)
{
    int ret = inBuff.readFd(fd, savedErrno);
    return ret;
}

int Request::write(int *savedErrno)
{
    int ret = outBuff.writeFd(fd, savedErrno);
    return ret;
}

void Request::appendOutBuffer(const Buffer &buf)
{
    outBuff.append(buf);
}

size_t Request::writeableBytes()
{
    return outBuff.readableBytes();
}

void Request::setTimer(Timer *timer)
{
    this->timer = timer;
}

Timer *Request::getTimer()
{
    return timer;
}

void Request::setWorking()
{
    working = true;
}

void Request::setNoWorking()
{
    working = false;
}

bool Request::isWorking() const
{
    return working;
}

bool Request::parseRequest()
{
    bool ok = true;
    bool hasMore = true;

    while (hasMore)
    {
        if (state == ExpectRequestLine)
        {
            const char *crlf = inBuff.findCRLF();
            if (crlf)
            {
                ok = parseRequestLine(inBuff.peek(), crlf);
                if (ok)
                {
                    inBuff.retrieveUntil(crlf + 2);
                    state = ExpectHeaders;
                }
                else
                {
                    hasMore = false;
                }
            }
            else
            {
                hasMore = false;
            }
        }
        else if (state == ExpectHeaders)
        {
            const char *crlf = inBuff.findCRLF();
            if (crlf)
            {
                const char *colon = std::find((const char *)inBuff.peek(), crlf, ':');

                if (colon != crlf)
                {
                    addHeader(inBuff.peek(), colon, crlf);
                }
                else
                {
                    state = GotAll;
                    hasMore = false;
                }
                inBuff.retrieveUntil(crlf + 2);
            }
            else
            {
                hasMore = false;
            }
        }
    }

    return ok;
}

bool Request::parseFinsh()
{
    return state = GotAll;
}

void Request::resetParse()
{
    state = ExpectRequestLine;
    method = Invalid;
    version = Unknown;
    path = "";
    query = "";
    headers.clear();
}

std::string Request::getPath() const
{
    return path;
}

std::string Request::getQuery() const
{
    return query;
}

std::string Request::getHeader(const std::string &field) const
{
    std::string res;
    auto it = headers.find(field);

    if (it != headers.end())
        ;
    res = it->second;

    return res;
}

std::string Request::getMethod() const
{
    std::string res;
    switch (method)
    {
    case Get:
        res = "GET";
        break;
    case Post:
        res = "POST";
        break;
    case Head:
        res = "HEAD";
        break;
    case Put:
        res = "PUT";
        break;
    case Delete:
        res = "DELETE";
    }

    return res;
}

bool Request::keepAlive() const
{
    std::string connection = getHeader("Connection");
    bool res = (connection == "Keep-Alive" || (version == HTTP11 && connection != "close"));

    return res;
}

Request::~Request()
{
    close(fd);
}
bool Request::parseRequestLine(const char *begin, const char *end)
{
    bool succeed = false;
    const char *start = begin;
    const char *space = std::find(start, end, ' ');
    if (space != end && setMethod(start, space))
    {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != space)
        {
            const char *question = std::find(start, space, '?');

            if (question != space)
            {
                setPath(start, question);
                setQuery(question, space);
            }
            else
            {
                setPath(start, space);
            }
            start = space + 1;
            succeed = (end - start == 8 && std::equal(start, end - 1, "HTTP/1."));

            if (succeed)
            {
                if (*(end - 1) == '1')
                    setVersion(HTTP11);
                else if (*(end - 1) == '0')
                    setVersion(HTTP10);
                else
                    succeed = false;
            }
        }
    }

    return succeed;
}

bool Request::setMethod(const char *begin, const char *end)
{
    std::string methodStr(begin, end);

    if (methodStr == "GET")
        method = Get;
    else if (methodStr == "POST")
        method = Post;
    else if (methodStr == "HEAD")
        method = Head;
    else if (methodStr == "PUT")
        method = Put;
    else if (methodStr == "DELETE")
        method = Delete;
    else
        method = Invalid;

    return method != Invalid;
}

void Request::setPath(const char *begin, const char *end)
{
    std::string subPath;
    subPath.assign(begin, end);
    if (subPath == "/")
        subPath = "/index.html";

    path = subPath;
}

void Request::setQuery(const char *begin, const char *end)
{
    query.assign(begin, end);
}

void Request::setVersion(Version version)
{
    this->version = version;
}

void Request::addHeader(const char *begin, const char *colon, const char *end)
{
    std::string field(begin, colon);
    ++colon;
    while (colon < end && *colon == ' ')
        ++colon;

    std::string value(colon, end);
    while (!value.empty() && value[value.size() - 1] == ' ')
        value.resize(value.size() - 1);

    headers[field] = value;
}