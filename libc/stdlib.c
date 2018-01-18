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
        for (p = strtok_r(path, " ", &last); p;
             p = strtok_r(NULL, " ", &last)) {
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

static inline int isdigit(int ch) {
	return (unsigned int)ch-'0' < 10;
}

static inline int isspace(int ch) {
	return ch == ' ' || (unsigned int)ch-'\t' < 5;
}

int atoi(const char *s) {
    int n = 0;
    int neg = 0;
    while (isspace(*s)) {
        s++;
    }
    switch (*s) {
        case '-':
            neg = 1;
        /* Fallthrough is intentional here */
        case '+':
            s++;
    }
    while (isdigit(*s)) {
        n = 10 * n - (*s++ - '0');
    }
    /* The sign order may look incorrect here but this is correct as n is
     * calculated
     * as a negative number to avoid overflow on INT_MAX.
     */
    return neg ? n : -n;
}
