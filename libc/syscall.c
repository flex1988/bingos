#include <errno.h>
#include <syscall.h>
#include <types.h>

DEFN_SYSCALL1(exit, SYSCALL_EXIT, int);
DEFN_SYSCALL1(println, SYSCALL_PRINTLN, const char *);
DEFN_SYSCALL1(printc, SYSCALL_PRINTC, char);
/*DEFN_SYSCALL1(printf, SYSCALL_PRINTF, const char *);*/
DEFN_SYSCALL3(execve, SYSCALL_EXECVE, const char *, char **, char **);
DEFN_SYSCALL0(fork, SYSCALL_FORK);
DEFN_SYSCALL0(getpid, SYSCALL_GETPID);
DEFN_SYSCALL1(waitpid, SYSCALL_WAITPID, int);
DEFN_SYSCALL1(brk, SYSCALL_BRK, const void *);
DEFN_SYSCALL3(open, SYSCALL_OPEN, const char *, int, int);
DEFN_SYSCALL3(read, SYSCALL_READ, int, void *, size_t);
DEFN_SYSCALL2(stat, SYSCALL_STAT,const char *, stat_t *);

int exit(int val) { return syscall_exit(val); }

int fork() { return syscall_fork(); }

int execve(const char *path, char **argv, char **envp) { return syscall_execve(path, argv, envp); }

int getpid() { return syscall_getpid(); }

int waitpid(int pid) { return syscall_waitpid(pid); }

void *brk(const void *addr) { return syscall_brk(addr); }

int open(const char *filename, int flags, int mode) { return syscall_open(filename, flags, mode); }

int read(int fd, void *buf, size_t count) { return syscall_read(fd, buf, count); }

int stat(const char *filename, stat_t *statbuf) { return syscall_stat(filename, statbuf); }

