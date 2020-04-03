#include "response.h"
#include "buffer.h"

#include <string>
#include <iostream>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

const std::map<int, std::string> Response::statusCode2Message = {
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"}};

const std::map<std::string, std::string> Response::suffix2Type = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/nsword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css"}};

Response::Response(int statusCode, std::string path, bool keepAlive)
    : statusCode(statusCode),
      path(path),
      keepAlive(keepAlive) {}
Buffer Response::makeResponse()
{
    Buffer output;

    if (statusCode == 400)
    {
        doErrorResponse(output, "server can't parse the message");
        return output;
    }

    struct stat sbuf;

    if (stat(path.data, &sbuf) < 0)
    {
        statusCode = 404;
        doErrorResponse(output, "server can't find the file");
        return output;
    }

    if (!(S_ISREG(sbuf.st_mode) || !(S_IRUSR & sbuf.st_mode)))
    {
        statusCode = 403;
        doErrorResponse(output, "server can't read the file");
        return output;
    }

    doStaticResquest(output, sbuf.st_size);
    return output;
}

void Response::doStaticResquest(Buffer &output, long fileSize)
{
    auto it = statusCode2Message.find(statusCode);

    if (it == statusCode2Message.end())
    {
        statusCode = 400;
        doErrorResponse(output, "Unknow status code");
        return;
    }

    output.append("HTTP/1.1 " + std::to_string(statusCode) + " " + it->second + "\r\n");

    if (keepAlive)
    {
        output.append("Connection: Keep-Alive\r\n");
        output.append("Keep-Alive: timeout=" + std::to_string(CONNECT_TIMEOUT) + "\r\n");
    }
    else
    {
        output.append("Connect: close\r\n");
    }

    output.append("Connect-type: " + getFileType() + "\r\n");
    output.append("Connect-length: " + std::to_string(fileSize) + "\r\n");
    output.append("Server: myServer\r\n");
    output.append("\r\n");

    int srcFd = open(path.data(), O_RDONLY, 0);

    void *mmapRet = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, srcFd, 0);
    close(srcFd);
    if (mmapRet == (void *)-1)
    {
        munmap(mmapRet, fileSize);
        output.retrieveAll();
        ;
        statusCode = 404;
        doErrorResponse(output, "server can't fild the file");
        return;
    }

    char *srcAddr = static_cast<char *>(mmapRet);
    output.append(srcAddr, fileSize);

    munmap(srcAddr, fileSize);
}

void Response::doErrorResponse(Buffer &output, std::string message)
{
    std::string body;
    auto it = statusCode2Message.find(statusCode);
    if (it == statusCode2Message.end())
        return;

    body += "<html><title>myServer Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    body += std::to_string(statusCode) + " : " + it->second + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>my web server</em></body></html>";

    output.append("HTTP/1.1 " + std::to_string(statusCode) + " " + it->second + "\r\n");

    output.append("Server: Swings\r\n");
    output.append("Content-type: text/html\r\n");
    output.append("Connection: close\r\n");
    output.append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");

    output.append(body);
}

std::string Response::getFileType()
{
    int idx = path.find_last_of('.');
    std::string suffix;
    if (idx == std::string::npos)
    {
        return "text/plain";
    }

    suffix = path.substr(idx);
    auto it = suffix2Type.find(suffix);

    if (it == suffix2Type.end())
    {
        return "text/plain";
    }

    return it->second;
}