#ifndef __SYSCALL_H__
#define __SYSCALL_H__

void syscalls_init();

#define DECL_SYSCALL0(fn) int syscall_##fn();
#define DECL_SYSCALL1(fn,p1) int syscall_##fn(p1);
#define DECL_SYSCALL2(fn,p1,p2) int syscall_##fn(p1,p2);
#define DECL_SYSCALL3(fn,p1,p2,p3) int syscall_##fn(p1,p2,p3);
#define DECL_SYSCALL4(fn,p1,p2,p3,p4) int syscall_##fn(p1,p2,p3,p4);
#define DECL_SYSCALL5(fn,p1,p2,p3,p4,p5) int syscall_##fn(p1,p2,p3,p4,p5);

#define DEFN_SYSCALL0(fn, num) \
int syscall_##fn() \
{ \
  int a; \
  __asm__ __volatile__("int $0x80" : "=a" (a) : "0" (num)); \
  return a; \
}

#define DEFN_SYSCALL1(fn,num,P1)   \
int syscall_##fn(P1 p1)  { \
    int a;  \
    __asm__ __volatile__("int $0x80" : "=a" (a) : "0"(num),"b"((int)p1));  \
    return a;   \
} 

#define DEFN_SYSCALL3(fn,num,P1,P2,P3)   \
int syscall_##fn(P1 p1,P2 p2,P3 p3)  { \
int a;  \
__asm__ __volatile__("int $0x80":"=a"(a):"0"(num),"b"((int)p1),"c"((int)p2),"d"((int)p3));  \
return a; } 


DECL_SYSCALL1(exit,int);
DECL_SYSCALL1(println,const char *);
DECL_SYSCALL3(execve,const char *,char **,char **);
DECL_SYSCALL0(fork);
DECL_SYSCALL0(getpid);
DECL_SYSCALL1(waitpid, int);
DECL_SYSCALL1(brk, const void *);

#define SYSCALL_EXIT 0
#define SYSCALL_PRINTLN 1
#define SYSCALL_EXECVE 7
#define SYSCALL_FORK 8
#define SYSCALL_GETPID 9
#define SYSCALL_WAITPID 10
#define SYSCALL_BRK 11

#endif
