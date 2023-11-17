#ifndef __INET_SOCK_H__
#define __INET_SOCK_H__

#if 0
static proto_ops_t inet_proto_ops = {
    AF_INET,         inet_create,   inet_dup,        inet_release,
    inet_bind,       inet_connect,  inet_socketpair, inet_accept,
    inet_getname,    inet_read,     inet_write,      inet_select,
    inet_ioctl,      inet_listen,   inet_send,       inet_recv,
    inet_sendto,     inet_recvfrom, inet_shutdown,   inet_setsockopt,
    inet_getsockopt, inet_fcntl,
};

void inet_proto_init() { ; }
#endif

#endif
