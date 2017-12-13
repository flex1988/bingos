#include <stdio.h>
#include <syscall.h>

int main(int argc, char **argv) {
    int i = 0;
    /*while (i++ < 1000);*/

    printf("Simple Shell.");

    int fd = open("/sh");
    printf("open file %d",fd);

    return 0;
}
