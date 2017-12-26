#include <errno.h>
#include <heap.h>
#include <kernel/memlayout.h>
#include <stat.h>
#include <stdio.h>
#include <stdlib.h>

static heap_t *heap = 0;

static void init_user_heap() { heap = create_heap(UHEAP_START, UHEAP_START + UHEAP_INITIAL_SIZE, 0x400000); }

void *malloc(size_t size) {
    if (!heap)
        init_user_heap();

    return _alloc(size, 0, heap);
}

void free(void *p) { _free(p, heap); }

int execvp(const char *file, char **argv) {
    if (file && file[0] != '/') {
        char *path = "/bin";
        int found = 0;
        char *p, *last;
        int i = 0;
        for (p = strtok_r(path, " ", &last); p; p = strtok_r(NULL, " ", &last)) {
            int ret;
            stat_t buf;
            char x[128];

            sprintf(x, "%s/%s", p, file);
            x[strlen(p) + strlen(file) + 2] = '\0';

            ret = stat(x, &buf);
            if (ret == -1) {
                free(x);
                continue;
            }

            return execve(x, NULL, NULL);
        }

        errno = ENOENT;
        return -1;

    } else if (file) {
        return execve(file, NULL, NULL);
    }

    errno = ENOENT;
    return -1;
}
