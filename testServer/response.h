#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include "server.h"

#include <string>
#include <map>

class Buffer;
class Response
{
public:
    static const std::map<int, std::string> statusCode2Message;
    static const std::map<std::string, std::string> suffix2Type;

    Response(int statusCode, std::string path, bool keepAlive);

    Buffer makeResponse();
    void doStaticResquest(Buffer &output, long fileSize);
    void doErrorResponse(Buffer &output, std::string message);

private:
    std::string getFileType();

private:
    int statusCode;   //响应状态码
    std::string path; //请求资源路径
    bool keepAlive;   //长连接
};

#endif