#include <errno.h>
#include <heap.h>
#include <stdlib.h>
#include <kernel/memlayout.h>

heap_t *heap = 0;

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
        char *p, *tokens[10], *last;
        int i = 0;
        for (p = strtok_r(path, " ", &last); p; p = strtok_r(NULL, " ", &last)) {
            int r;
            // stat_t stat;
            char *x = malloc(strlen(p) + strlen(file) + 2);
            strcpy(x, p);
            strcat(x, "/");
            strcat(x, file);
            
            println(x);
        }
    } else if (file) {
        ;
    }

    errno = ENOENT;
    return 1;
}
