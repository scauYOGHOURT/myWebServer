#include "utils.h"

#include <iostream>
#include <cstring>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int createListenFd(int port)
{
    port = (port <= 1024 || port >= 65536) ? 8888 : port;

    int listenFd = 0;
    if ((listenFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
    {
        printf("[createListenFd] fd = %d socket : %s\n", listenFd, strerror(errno));
        return -1;
    }

    int optval = 1;
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) == -1)
    {
        printf("[createListenFd] fd = %d setsockopt : %s\n", listenFd, strerror(errno));
        return -1;
    }

    struct sockaddr_in serverAddr;
    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    if (bind(listenFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        printf("[createListenFd]fd = %d bind : %s\n", listenFd, strerror(errno));
        return -1;
    }

    if (listen(listenFd, LISTENQ) == -1)
    {
        printf("[createListenFd] fd = %d listen : %s\n", listenFd, strerror(errno));
        return -1;
    }

    if (listenFd == -1)
    {
        close(listenFd);
        return -1;
    }

    return listenFd;
}

int setNonBlocking(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    if (flag == -1)
    {
        printf("[setNoBlocking]fd  = %d fcntl : %s\n", fd, strerror(errno));
        return -1;
    }

    flag |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flag) == -1)
    {
        printf("[setNonBlocking]fd = %d fcntl: %s\n", fd, strerror(errno));
        return -1;
    }

    return 0;
}