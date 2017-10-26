#include "kernel/elf.h"
#include "kernel.h"

bool_t elf_ehdr_check(elf32_ehdr *ehdr) {
    if (*(uint32_t *)ehdr->e_ident != ELF_MAGIC) {
        printk("invalid elf magic");
        return false;
    }

    if (ehdr->e_ident[ELF_EI_VERSION] != 1 || ehdr->e_version != 1) {
        printk("invalid elf version");
        return false;
    }

    if (ehdr->e_ident[ELF_EI_CLASS] != ELF_CLASS32 || ehdr->e_ident[ELF_EI_DATA] != ELF_LITTLE_ENDIAN || ehdr->e_machine != EM_386)
        return false;

    return true;
}
