#ifndef __UTILS_H__
#define __UTILS_H__

#define LISTENQ 1024

int createListenFd(int port);
int setNonBlock(int fd);
#endif