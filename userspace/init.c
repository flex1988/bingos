#include <stdio.h>
#include <syscall.h>
#include <types.h>

int main(int argc, char **argv) {
    printf("Init process started %d.", getpid());
    int pid;

    if ((pid = fork()) == 0) {
        printf("shell xxxxxxxxxxxxxx %d", pid);
        execve("/sh", 0, 0);
        exit(0);
    } else {
        printf("parent xxxxxxxxxx %d", pid);
    }

    while (1)
        ;
}
