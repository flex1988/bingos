#ifndef __SOCKET_H__
#define __SOCKET_H__

#define SYS_SOCKET 1
#define SYS_BIND 2
#define SYS_CONNECT 3
#define SYS_LISTEN 4

int socket(int domain, int type, int protocol);

#endif
