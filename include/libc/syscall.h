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
  asm volatile("int $0x80" : "=a" (a) : "0" (num)); \
  return a; \
}

#define DEFN_SYSCALL1(fn,num,P1)   \
int syscall_##fn(P1 p1)  { \
int a;  \
asm volatile("int $0x80":"=a"(a):"0"(num),"b"((int)p1));  \
return a; }  \


DECL_SYSCALL0(say);
DECL_SYSCALL1(exit,int);
DECL_SYSCALL1(println,const char *);
#endif
