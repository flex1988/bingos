#include <syscall.h>

DEFN_SYSCALL1(println, 0, const char *);

DEFN_SYSCALL3(exec, 1, char *, int, char **);

DEFN_SYSCALL0(fork, 3);

DEFN_SYSCALL1(exit, 5, int);

int exit(int val) { return syscall_exit(val); }

int fork() { return syscall_fork(); }

int exec(char *path, int argc, char **argv) { return syscall_exec(path, argc, argv); };
