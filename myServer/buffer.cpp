#include "buffer.h"

#include <cstdio>
#include <cstring>
#include <iostream>

#include <sys/uio.h>
#include <unistd.h>
using namespace wings;

int Buffer::append(const char* buf, int len){
    if(!isEnough(len)){
        _makeSpace(len);
    }

    char* start = writer();
    ::memcpy(start, buf, len);
    writerIndex += len;
    
    return 1;
}

int Buffer::readFd(int fd, int* err){
    char exbuff[65536];

    struct iovec vec[2];
    int writeableBytes =  writeable();
    vec[0].iov_base = writer();
    vec[0].iov_len = writeableBytes;

    vec[1].iov_base = exbuff;
    vec[1].iov_len = sizeof(exbuff);

    const int n = readv(fd, vec, 2);
    if(n < 0){
        *err = errno;
    }else if(n <= writeable()){
        writerIndex += n;
    }else{
        writerIndex = buff.size();
        append(exbuff, n - writeableBytes);
    }

    return n;
}

int Buffer::writeFd(int fd, int* err){
    int nLeft = writeable();
    char* start = reader();

    int n = write(fd, start, nLeft);

    if(n < 0){
        *err = errno;
    }
    else{
        readerIndex += n;
    }

    return n;
}

void Buffer::_makeSpace(int len){
    if(writeable() + readerIndex < len){
        buff.resize(writerIndex + len);
    }else{
        int readableBytes = readable();
        memcpy(_peek(), reader(), readableBytes);

        readerIndex = 0;
        writerIndex = readableBytes;
    }

}

