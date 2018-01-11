#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <types.h>
#include <stat.h>
#include <errno.h>
#include <dirent.h>

void syscalls_init();

#define DECL_SYSCALL0(fn) int syscall_##fn();
#define DECL_SYSCALL1(fn,p1) int syscall_##fn(p1);
#define DECL_SYSCALL2(fn,p1,p2) int syscall_##fn(p1,p2);
#define DECL_SYSCALL3(fn,p1,p2,p3) int syscall_##fn(p1,p2,p3);
#define DECL_SYSCALL4(fn,p1,p2,p3,p4) int syscall_##fn(p1,p2,p3,p4);
#define DECL_SYSCALL5(fn,p1,p2,p3,p4,p5) int syscall_##fn(p1,p2,p3,p4,p5);

#define DEFN_SYSCALL0(fn, num) \
    int syscall_##fn() { \
        int a; \
        __asm__ __volatile__("int $0x80" : "=a" (a) : "0" (num)); \
        if(a >= 0) \
            return a;   \
        errno = -a ;    \
        return -1;  \
    }

#define DEFN_SYSCALL1(fn,num,P1)   \
    int syscall_##fn(P1 p1)  { \
        int a;  \
        __asm__ __volatile__("int $0x80" : "=a" (a) : "0"(num),"b"((int)p1));  \
        if(a >= 0)   \
            return a;   \
        errno = -a; \
        return -1;   \
    }

#define DEFN_SYSCALL2(fn,num,P1,P2) \
    int syscall_##fn(P1 p1,P2 p2)   {   \
        int a;  \
        __asm__ __volatile__("int $0x80": "=a"(a) : "0"(num),"b"((int)p1),"c"((int)p2)); \
        if(a >= 0)   \
            return a;   \
        errno = -a; \
        return -1;   \
    }

#define DEFN_SYSCALL3(fn,num,P1,P2,P3)   \
    int syscall_##fn(P1 p1,P2 p2,P3 p3)  { \
        int a;  \
        __asm__ __volatile__("int $0x80":"=a"(a):"0"(num),"b"((int)p1),"c"((int)p2),"d"((int)p3));  \
        if(a >= 0)   \
            return a;   \
        errno = -a; \
        return -1;   \
    } 


DECL_SYSCALL1(exit,int);
DECL_SYSCALL1(println,const char *);
DECL_SYSCALL1(printc,char);
DECL_SYSCALL1(printf,const char *);
DECL_SYSCALL3(execve,const char *,char **,char **);
DECL_SYSCALL0(fork);
DECL_SYSCALL0(getpid);
DECL_SYSCALL1(waitpid, int);
DECL_SYSCALL1(brk, const void *);
DECL_SYSCALL3(open,const char*,int,int);
DECL_SYSCALL3(read,int,void *,size_t);
DECL_SYSCALL2(stat,const char *,stat_t *);
DECL_SYSCALL1(close,int);
DECL_SYSCALL3(readdir,int,int,dirent_t *);
DECL_SYSCALL2(socketcall,int,int *);
DECL_SYSCALL2(gettimeofday,timeval_t *,void *);

void *sbrk(int increment);

#define SYSCALL_EXIT 0
#define SYSCALL_PRINTLN 1
#define SYSCALL_OPEN 2
#define SYSCALL_READ 3
#define SYSCALL_WRITE 4 
#define SYSCALL_CLOSE 5
#define SYSCALL_PRINTC 6
#define SYSCALL_EXECVE 7
#define SYSCALL_FORK 8
#define SYSCALL_GETPID 9
#define SYSCALL_WAITPID 10
#define SYSCALL_BRK 11
#define SYSCALL_STAT 12
#define SYSCALL_READDIR 13
#define SYSCALL_SOCKETCALL 14
#define SYSCALL_GETTIMEOFDAY 15

#endif
