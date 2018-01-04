#include <errno.h>
#include <heap.h>
#include <kernel/memlayout.h>
#include <stat.h>
#include <stdio.h>
#include <stdlib.h>

int execvp(const char *file, char **argv) {
    if (file && file[0] != '/') {
        char *path = "/bin";
        int found = 0;
        char *p, *last;
        int i = 0;
        for (p = strtok_r(path, " ", &last); p; p = strtok_r(NULL, " ", &last)) {
            int ret;
            stat_t buf;
            char *x = malloc(128);
            memset(x, 0x0, 128);

            sprintf(x, "%s/%s", p, file);

            ret = stat(x, &buf);

            if (ret == -1) {
                /*free(x);*/
                continue;
            }

            return execve(x, argv, NULL);
        }

        errno = ENOENT;
        return -1;

    } else if (file) {
        return execve(file, argv, NULL);
    }

    errno = ENOENT;
    return -1;
}
