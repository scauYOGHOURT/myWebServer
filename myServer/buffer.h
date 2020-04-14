#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <vector>

#define BUFFER_SIZE 1024
namespace wings
{

class Buffer
{
public:
    Buffer():buff(BUFFER_SIZE), writerIndex(0), readerIndex(0){};
    ~Buffer(){}


    int readable() { return writerIndex - readerIndex;}
    int writeable() { return buff.size() - writerIndex;}
    char* writer() { return _peek() + writerIndex; }
    char* reader() { return _peek() + readerIndex; }

    int append(const char* buf, int len);
    int append(Buffer* other){return append(other->reader(), other->readable()); };

    bool isEnough(int len){ return writeable() >= len;}
    
    int readFd(int fd, int* err);
    int writeFd(int fd, int* err);
private:
    void _makeSpace(int len);
    char *_peek() { return &*buff.begin(); }

private:
    std::vector<char> buff;
    int readerIndex;
    int writerIndex;
};
} // namespace wings

#endif