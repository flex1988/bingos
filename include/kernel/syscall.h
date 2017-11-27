#ifndef __SYSCALL_H__
#define __SYSCALL_H__
int sys_exit(int ret);
int sys_println(const char *);
extern int sys_exec(char *path, int argc, char **argv);
int sys_open(const char *path, int oflag, ...);
int sys_read(int fd, void *buf, size_t nbytes);
int sys_write(int fd, const void *buf, size_t nbytes);
int sys_close(fd);
int sys_gettimeofday();
int sys_execve();
extern int sys_fork();
extern int sys_getpid();
extern void *sys_brk();
extern int sys_waitpid(int pid);
#endif
