#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <vector>
#include <string>
#include <algorithm> // copy
#include <iostream>

#define BUFFER_SIZE 1024

class Buffer
{
public:
    Buffer();

    size_t readableBytes() const;  //可读字节
    size_t writeableBytes() const; //可写字节
    size_t prependableBytes() const;
    char *peek();
    const char *peek() const;                  //可读位置
    void retrieve(size_t len);                 //取出len个字节
    void retrieveUntil(const char *end);       //取出数据到end
    void retrieveAll();                        //取出buffer内全部数据
    std::string retrieveAsString();            //以string形式取出数据
    void append(const std::string &str);       //插入数据
    void append(const char *data, size_t len); //插入数据
    void append(const void *data, size_t len); //插入数据
    void append(const Buffer &otherBuff);      //读入其他缓冲区的数据
    void ensureWritableBytes(size_t len);      //确认缓冲区有足够空间
    char *beginWrite();                        //可写指针
    const char *beginWrite() const;
    void hasWritten(size_t len);              //移动index
    ssize_t readFd(int fd, int *saveErrno);   //从套接字读入
    ssize_t writeFd(int fd, int *savedErrno); //写入套接字
    const char *findCRLF() const;
    const char *findCRLF(const char *start) const;

    ~Buffer() {}

private:
    char *begin(); //返回头指针
    const char *begin() const;
    void makeSpace(size_t len);

private:
    std::vector<char> buffer;
    size_t readerIndex;
    size_t writerIndex;
};

#endif