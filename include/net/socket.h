#ifndef __SOCKET_H__
#define __SOCKET_H__

typedef struct sockaddr {
    uint16_t sa_family;
    char sa_data[14];
} sockaddr_t;

#endif
