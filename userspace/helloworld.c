#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>
#include <stat.h>
#include <socket.h>

int main () {
    char *a = malloc(1000);
    memset(a,0x0,1000);
    sprintf(a,"hello,world!");
    println("a 0x%x",a);
    println(a);

    free(a);

    printf("hello world!\n");

    return 0;
}
