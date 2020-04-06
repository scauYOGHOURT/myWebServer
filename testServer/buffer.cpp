#include "buffer.h"

#include <cstring>
#include <iostream>

#include <unistd.h>
#include <sys/uio.h>

Buffer::Buffer() : buffer(BUFFER_SIZE), readerIndex(0), writerIndex(0) {}

size_t Buffer::readableBytes() const
{
    return writerIndex - readerIndex;
}

size_t Buffer::writeableBytes() const
{
    return buffer.size() - writerIndex;
}

size_t Buffer::prependableBytes() const
{
    return readerIndex;
}

char *Buffer::peek()
{
    return begin() + readerIndex;
}

const char *Buffer::peek() const
{
    return begin() + readerIndex;
}

void Buffer::retrieve(size_t len)
{
    //printf("%d\n", len);
    readerIndex += std::min(len, readableBytes());
}

void Buffer::retrieveUntil(const char *end)
{
    retrieve(end - peek());
}

void Buffer::retrieveAll()
{
    readerIndex = 0;
    writerIndex = 0;
}

std::string Buffer::retrieveAsString()
{
    std::string str(peek() + readableBytes());
    retrieveAll();

    return str;
}

void Buffer::append(const std::string &str)
{
    append(str.data(), str.length());
}

void Buffer::append(const char *data, size_t len)
{
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
}

void Buffer::append(const void *data, size_t len)
{
    append(static_cast<const char *>(data), len);
}

void Buffer::append(const Buffer &otherBuff)
{
    append(otherBuff.peek(), otherBuff.readableBytes());
}

void Buffer::ensureWritableBytes(size_t len)
{
    if (writeableBytes() < len)
        makeSpace(len);
}

char *Buffer::beginWrite()
{
    return begin() + writerIndex;
}

const char *Buffer::beginWrite() const
{
    return begin() + writerIndex;
}

void Buffer::hasWritten(size_t len)
{
    writerIndex += len;
}

ssize_t Buffer::readFd(int fd, int *savedErrno)
{
    char exbuff[65536];
    struct iovec vec[2];
    const size_t writeable = writeableBytes();
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writeableBytes();

    vec[1].iov_base = exbuff;
    vec[1].iov_len = sizeof(exbuff);

    const ssize_t n = readv(fd, vec, 2);

    if (n < 0)
    {
        fprintf(stderr, "[Buffer:readFd]fd = %d readv: %s\n", fd, strerror(errno));
        *savedErrno = errno;
    }
    else if (static_cast<size_t>(n) <= writeable)
    {
        writerIndex += n;
    }
    else
    {
        writerIndex = buffer.size();
        append(exbuff, n - writeable);
    }

    return n;
}

ssize_t Buffer::writeFd(int fd, int *savedErrno)
{

    size_t nLeft = readableBytes();
    char *startPtr = peek();
    ssize_t n;

    if ((n = write(fd, startPtr, nLeft)) < 0)
    {
        if (n < 0 && n == EINTR)
        {
            return 0;
        }
        else
        {
            fprintf(stderr, "[Buffer:writeFd]fd = %d write : %s\n", fd, strerror(errno));
            *savedErrno = errno;
            return -1;
        }
    }
    else
    {
        readerIndex += n;
        return n;
    }
}

const char *Buffer::findCRLF() const
{
    const char CRLF[] = "\r\n";
    const char *crlf = std::search(peek(), beginWrite(), CRLF, CRLF + 2);

    return crlf == beginWrite() ? nullptr : crlf;
}

const char *Buffer::findCRLF(const char *start) const
{
    if (peek() > start)
        return nullptr;
    if (start > beginWrite())
        return nullptr;

    const char CRLF[] = "\r\n";
    const char *crlf = std::search(start, beginWrite(), CRLF, CRLF + 2);

    return crlf == beginWrite() ? nullptr : crlf;
}

char *Buffer::begin()
{
    return &*buffer.begin();
}

const char *Buffer::begin() const
{
    return &*buffer.begin();
}

void Buffer::makeSpace(size_t len)
{
    if (writeableBytes() + prependableBytes() < len)
    {
        buffer.resize(writerIndex + len);
    }
    else
    {
        size_t readable = readableBytes();

        std::copy(peek(), beginWrite(), begin());

        readerIndex = 0;
        writerIndex = readable;
    }
}