#include "fs/fs.h"
#include "kernel.h"
#include "kernel/elf.h"
#include "kernel/memlayout.h"
#include "kernel/mm.h"
#include "kernel/process.h"

extern process_t *_current_process;

static int do_exec(char *path, int argc, char **argv) {
    int ret = -1;
    vfs_node_t *n;
    elf32_ehdr *ehdr;

    n = vfs_lookup(path, 0);

    ASSERT(n);

    ptr_t virt;
    ptr_t entry;
    page_t *page;

    ret = do_mmap(USTACK_BOTTOM, n->length);
    ASSERT(!ret);

    ehdr = (elf32_ehdr *)USTACK_BOTTOM;

    ret = vfs_read(n, 0, n->length, (uint8_t *)ehdr);
    ASSERT(ret >= sizeof(elf32_ehdr));

    if (!elf_ehdr_check(ehdr)) {
        printk("invalid elf header");
        return -1;
    }

    if (!elf_load_sections(ehdr)) {
        printk("load elf sections error");
        return -1;
    }

    entry = ehdr->e_entry;

    ret = do_munmap(USTACK_BOTTOM, n->length);
    ASSERT(!ret);

    ret = do_mmap(USTACK_BOTTOM, USTACK_SIZE);
    ASSERT(!ret);

    ret = do_mmap(UHEAP_START, UHEAP_INITIAL_SIZE);
    ASSERT(!ret);

    _current_process->brk = UHEAP_START;
    _current_process->ustack = USTACK_BOTTOM + USTACK_SIZE;

    switch_to_user_mode(entry, _current_process->ustack);
}

int sys_exec(char *path, int argc, char **argv) {
    int ret;
    ret = do_exec(path, argc, argv);
    return ret;
}
