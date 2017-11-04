#include "kernel/elf.h"
#include "kernel.h"
#include "kernel/mmu.h"

extern page_dir_t *_current_pd;

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

bool_t elf_load_sections(elf32_ehdr *ehdr) {
    uint32_t virt;
    elf32_shdr *shdr;
    page_t *page;

    for (virt = 0; virt < (ehdr->e_shentsize * ehdr->e_shnum); virt += ehdr->e_shentsize) {
        shdr = (elf32_shdr *)((uint8_t *)ehdr + (ehdr->e_shoff + virt));

        if (shdr->sh_addr) {
            for (virt = 0; virt < (shdr->sh_size + 0x2000); virt += PAGE_SIZE) {
                page = get_page(shdr->sh_addr + virt, 1, _current_pd);
                ASSERT(page);
                page_map(page, 0, 1);
            }
        }

        switch (shdr->sh_type) {
            case SHT_NOBITS:
                memset((void *)shdr->sh_addr, 0, shdr->sh_size);
                break;

            case SHT_PROGBITS:
            case SHT_STRTAB:
            case SHT_SYMTAB:
                memcpy((void *)shdr->sh_addr, (uint8_t *)ehdr + shdr->sh_offset, shdr->sh_size);
                break;
        }
    }

    return true;
}
