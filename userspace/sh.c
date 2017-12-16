#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <types.h>

int main(int argc, char **argv) {
    printf("Simple Shell.");
    char *buf = malloc(1024);
    while (1) {
        int n = read(0, buf, 1);
        printf("read: %s", buf);
    }
    return 0;
}
