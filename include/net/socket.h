#ifndef __SOCKET_H__
#define __SOCKET_H__

typedef struct socket {
    int16_t type;
    int32_t flags;
    void* data;
    struct socket* conn;
    struct socket* iconn;
    struct socket* next;
    struct proto_ops* ops;
    struct vfs_node_s* inode;
} socket_t;

typedef struct sockaddr {
    uint16_t sa_family;
    char sa_data[14];
} sockaddr_t;

#endif
