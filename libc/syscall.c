#include <syscall.h>

DEFN_SYSCALL1(exit, 5, int);
DEFN_SYSCALL1(println, 0, const char *);

int exit(int val) { return syscall_exit(val); }
