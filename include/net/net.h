#ifndef __NET_H__
#define __NET_H__

#include "fs/fs.h"
#include "net/socket.h"

#include <types.h>

#define NPROTO 16

#define SYS_SOCKET 1

typedef struct proto_ops {
    int family;
    int (*create)(socket_t* sock, int protocol);
    int (*dup)(socket_t* nsock, socket_t* osock);
    int (*release)(socket_t* sock, socket_t* peer);
    int (*bind)(socket_t* sock, sockaddr_t* umyaddr, int sockaddr_len, int flags);
    int (*connect)(socket_t* sock, sockaddr_t* uservaddr, int sockaddr_len, int flags);
    int (*socketpair)(socket_t* sock1, socket_t* sock2);
    int (*accept)(socket_t* sock, socket_t* newsock, int flags);
    int (*getname)(socket_t* sock, sockaddr_t* uaddr, int* usockaddr_len, int peer);
    int (*read)(socket_t* sock, char* ubuf, int size, int nonblock);
    int (*write)(socket_t* sock, char* ubuf, int size, int nonblock);
    int (*select)(socket_t* sock, int sel_type, void* wait);
    int (*ioctl)(socket_t* sock, uint32_t cmd, uint32_t arg);
    int (*listen)(socket_t* sock, int len);
    int (*send)(socket_t* sock, void* buff, int len, int nonblock, uint32_t flags);
    int (*recv)(socket_t* sock, void* buff, int len, int nonblock, uint32_t flags);
    int (*sendto)(socket_t* sock, void* buff, int len, int nonblock, uint32_t flags, sockaddr_t* addr, int addr_len);
    int (*recvfrom)(struct socket* sock, void* buff, int len, int nonblock, uint32_t flags, sockaddr_t* addr, int* addr_len);
    int (*shutdown)(socket_t* sock, int flags);
    int (*setsockopt)(socket_t* sock, int level, int optname, char* optval, int* optlen);
    int (*getsockopt)(socket_t* sock, int level, int optname, char* optval, int* optlen);
    int (*fcntl)(socket_t* sock, uint32_t cmd, uint32_t arg);
} proto_ops_t;

void sock_init(void);
int sock_register(int family, proto_ops_t *ops);

void init_netif_funcs();
#endif
