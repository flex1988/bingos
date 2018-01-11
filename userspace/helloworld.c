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

    timeval_t tv;
    gettimeofday(&tv, NULL);

    int hour = (tv.tv_sec / 3600) % 24;
    int minutes = (tv.tv_sec / 60) % 60;
    int seconds = tv.tv_sec % 60;

    println("%02d:%02d:%02d", hour, minutes, seconds);

    

    return 0;
}
