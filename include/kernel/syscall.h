#ifndef __SYSCALL_H__
#define __SYSCALL_H__
int sys_exit(int ret);
int sys_println(const char *);
int sys_printc(char c);
extern int sys_exec(char *path, int argc, char **argv);
extern int sys_open(const char *filename, int flags, int mode);
extern int sys_read(int fd, void *buf, size_t nbytes);
int sys_write(int fd, const void *buf, size_t nbytes);
extern int sys_close(fd);
int sys_gettimeofday();
int sys_execve();
extern int sys_fork();
extern int sys_getpid();
extern void *sys_brk();
extern int sys_waitpid(int pid);
extern int sys_stat(char *filename, vfs_stat_t *statbuf);
extern int sys_readdir(int fd,int index,dirent_t *dir);
#endif
