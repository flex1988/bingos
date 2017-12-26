#include <stdio.h>
#include <syscall.h>
#include <stat.h>

int main () {
    stat_t buf;
    memset(&buf,0x0,sizeof(stat_t));
    stat("/bin/helloworld",&buf);
    println("buf->st_ino %x",buf.st_ino);
    printf("hello world!\n");
    return 0;
}
