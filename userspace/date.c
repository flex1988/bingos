#include <syscall.h>
#include <time.h>

int main() {
    timeval_t tv;
    gettimeofday(&tv, NULL);

    struct tm tm;
    gmtime_r(&tv.tv_sec, &tm);

    char buf[32];// = malloc(32);

    strftime(buf, 32, "%c\n", &tm);

    println(buf);

    /*free(buf);*/

    return 0;
}
