#include <stdio.h>
#include <syscall.h>
#include <types.h>

int main(int argc, char **argv) {
    printf("Init process started.");

    int pid = fork();

    /*if (pid == 0) {*/
        /*exec("/sh", 0, 0);*/
    /*} else {*/
        /*printf("parent");*/
    /*}*/
}
