#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <types.h>

int main(int argc, char **argv) {
    printf("Init process started %d.", getpid());
    int pid;

    if ((pid = fork()) == 0) {
        void *p = malloc(1000);
        memcpy(p, "hello world!", sizeof("hello world!"));
        printf(p);
        free(p);
        execve("/bin/sh", 0, 0);
        exit(0);
    } else {
        printf("parent xxxxxxxxxx %d", pid);
    }

    while (1)
        ;
}
