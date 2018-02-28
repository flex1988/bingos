#include <stdio.h>
#include <dirent.h>
#include <syscall.h>


int main() {
    int ret = mkdir("/helloworld", 0);
    if (ret != 0)
        println("mkdir error %d", ret);
    return 0;
}
