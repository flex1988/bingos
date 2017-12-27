#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>
#include <stat.h>

int main () {
    /*stat_t buf;*/
    /*memset(&buf,0x0,sizeof(stat_t));*/
    /*stat("/bin/helloworld",&buf);*/
    /*println("buf->st_ino %x",buf.st_ino);*/
    char *a = malloc(1000);
    memset(a,0x0,1000);
    sprintf(a,"hello,world!");
    println("a 0x%x",a);
    println(a);

    free(a);

    printf("hello world!\n");
    return 0;
}
