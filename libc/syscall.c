#include <syscall.h>

DEFN_SYSCALL1(exit, SYSCALL_EXIT, int);
DEFN_SYSCALL1(println, SYSCALL_PRINTLN, const char *);
DEFN_SYSCALL3(execve, SYSCALL_EXECVE, const char *, char **, char **);
DEFN_SYSCALL0(fork, SYSCALL_FORK);
DEFN_SYSCALL0(getpid, SYSCALL_GETPID);
DEFN_SYSCALL1(waitpid, SYSCALL_WAITPID, int);

int exit(int val) { return syscall_exit(val); }

int fork() {
    int pid = syscall_fork();
    return pid;
}

int execve(const char *path, char **argv, char **envp) { return syscall_execve(path, argv, envp); }

int getpid() { return syscall_getpid(); }

int waitpid(int pid) { return syscall_waitpid(pid); }
