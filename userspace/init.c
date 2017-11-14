#include <stdio.h>
#include <syscall.h>
#include <types.h>

int main(int argc, char **argv) {
    printf("Init process started.");
    int pid;

    if (!(pid = fork())) {
        execve("/sh", 0, 0);
        exit(0);
    }

    while (1) {
        ;
    }
}
