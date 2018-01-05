#include <socket.h>

int socket(int domain, int type, int protocol) { return syscall_socketcall(SYS_SOCKET, &domain); }
