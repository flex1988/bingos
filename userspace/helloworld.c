#include <socket.h>
#include <stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <types.h>

int main() {
    char *a = malloc(1000);
    memset(a, 0x0, 1000);
    sprintf(a, "hello,world!");
    println(a);
    free(a);
    return 0;
}
