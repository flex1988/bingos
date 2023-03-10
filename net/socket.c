#include "kernel.h"
#include "kernel/process.h"
#include "net/net.h"

#include <errno.h>
#include <socket.h>

static int sock_lseek(vfs_node_t *inode, uint32_t offset, int whence);
static int sock_read(vfs_node_t *inode, char *buf, int size);
static int sock_write(vfs_node_t *inode, char *buf, int size);
static int sock_readdir(vfs_node_t *inode, dirent_t *dir, int count);
static int sock_close(vfs_node_t *inode);
static int sock_select(vfs_node_t *inode, int sel_type, void *wait);
static int sock_ioctl(vfs_node_t *inode, uint32_t cmd, uint32_t arg);

static proto_ops_t *pops[NPROTO];

static void sock_release(socket_t *sock) {}

int sock_close(vfs_node_t *node) { sock_release(node->socket); }

static int get_fd(vfs_node_t *node) {
    int fd;

    for (fd = 0; fd < NR_OPEN; ++fd)
        if (!CP->fds->entries[fd])
            break;

    if (fd == NR_OPEN)
        return -1;

    CP->fds->entries[fd] = node;

    node->read = sock_read;
    node->write = sock_write;
    node->readdir = sock_readdir;
    // node->select = sock_select;
    node->close = sock_close;

    return fd;
}

static socket_t *sock_alloc() {
    socket_t *sock = kmalloc(sizeof(socket_t));
    sock->flags = 0;
    // sock->ops = NULL;
    sock->data = NULL;
    sock->conn = NULL;
    sock->iconn = NULL;
    sock->next = NULL;

    vfs_node_t *inode = kmalloc(sizeof(vfs_node_t));
    inode->flags = VFS_SOCKET;
    inode->uid = CP->uid;
    inode->gid = CP->gid;
    inode->socket = sock;

    sock->inode = inode;

    return sock;
}

static int sock_socket(int family, int type, int protocol) {
    int i, fd;
    socket_t *sock;

    sock = sock_alloc();

    return 0;
}

static int sock_lseek(vfs_node_t *inode, uint32_t offset, int whence) { return -ESPIPE; }

static int sock_read(vfs_node_t *inode, char *ubuf, int size) {
    socket_t *sock = inode->socket;
    return sock->ops->read(sock, ubuf, size, 0);
}

static int sock_write(vfs_node_t *inode, char *ubuf, int size) {
    socket_t *sock = inode->socket;
    return sock->ops->write(sock, ubuf, size, 0);
}

static int sock_readdir(vfs_node_t *inode, dirent_t *dir, int count) { return -EBADF; }

static int sock_select(vfs_node_t *inode, int sel_type, void *wait) {
    socket_t *sock = inode->socket;
    return inode->socket->ops->select(sock, sel_type, wait);
}

int sys_socketcall(int call, int *args) {
    switch (call) {
        case SYS_SOCKET:
            return sock_socket(*args, *(args + 1), *(args + 2));

        default:
            return -EINVAL;
    }
}

int sock_register(int family, proto_ops_t *ops) {
    int i;

    IRQ_OFF;
    for (i = 0; i < NPROTO; i++) {
        if (pops[i] != NULL)
            continue;
        pops[i] = ops;
        pops[i]->family = family;

        IRQ_ON;
        return i;
    }

    IRQ_RES;
    return -ENOMEM;
}

void sock_init(void) {
    int i;
    for (i = 0; i < NPROTO; i++) pops[i] = NULL;
    printk("[Network] Network init...");
}
