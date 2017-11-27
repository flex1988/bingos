#include <stdio.h>
#include <syscall.h>
#include <types.h>

int main(int argc, char **argv) {
    printf("Init process started %d.", getpid());
    int pid;

    if ((pid = fork()) == 0) {
        printf("shell xxxxxxxxxxxxxx %d", pid);
        uint32_t initial_brk = brk(0);
        printf("initial brk 0x%x", initial_brk);
        uint32_t brk2 = brk(initial_brk + 0x10000);
        printf("brk 0x%x", brk2);
        execve("/sh", 0, 0);
        exit(0);
    } else {
        printf("parent xxxxxxxxxx %d", pid);
    }

    while (1)
        ;
}
