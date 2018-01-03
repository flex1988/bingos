#include <stdio.h>
#include <syscall.h>
#include <types.h>

int main(int argc, char **argv) {
    if(!argv[1]) {
        println("Invalid arguments.");
        return 0;
    }

    char *target = argv[1];
    stat_t buf;
    stat(target,&buf);
    println("nid %d",buf.st_ino);
    
    return 0;
}
