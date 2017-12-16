#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <types.h>

int main(int argc, char **argv) {
    printf("Init process started %d.\n", getpid());
    int pid;

    if ((pid = fork()) == 0) {
        execve("/bin/sh", 0, 0);
    }

    while (1)
        ;
}
