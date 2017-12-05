#include "module.h"
#include "kernel.h"
#include "kernel/elf.h"

void *module_load(void *blob, size_t length) {
    elf32_ehdr *ehdr = (elf32_ehdr *)blob;
    char *head = (char *)blob;

    if (!elf_ehdr_check(ehdr)) {
        return NULL;
    }

    char *shstrtab = NULL;   // section header string table
    char *symstrtab = NULL;  // symbol string table
    elf32_shdr *sym_shdr = NULL;
    char *deps = NULL;
    size_t deps_length = 0;

    {
        uint32_t i = 0;
        for (uint32_t x = 0; x < ehdr->e_shentsize * ehdr->e_shnum; x += ehdr->e_shentsize) {
            elf32_shdr *shdr = (elf32_shdr *)((uint32_t)ehdr + ehdr->e_shoff + x);
            if (i == ehdr->e_shstrndx) {
                shstrtab = (char *)((uint32_t)ehdr + shdr->sh_offset);
            }
            i++;
        }
    }

    if (!shstrtab) {
        printk("can not locate module section header string table");
        return NULL;
    }

    {
        for (uint32_t x = 0; x < ehdr->e_shentsize * ehdr->e_shnum; x += ehdr->e_shentsize) {
            elf32_shdr *shdr = (elf32_shdr *)((uint32_t)ehdr + ehdr->e_shoff + x);
            if (shdr->sh_type == SHT_STRTAB && (!strcmp((char *)shstrtab + shdr->sh_name, ".strtab"))) {
                symstrtab = (char *)((uint32_t)ehdr + shdr->sh_offset);
            }
        }
    }

    if (!symstrtab) {
        printk("can not locate module symbol string table");
        return NULL;
    }

    {
        for (uint32_t x = 0; x < ehdr->e_shentsize * ehdr->e_shnum; x += ehdr->e_shentsize) {
            elf32_shdr *shdr = (elf32_shdr *)((uint32_t)ehdr + ehdr->e_shoff + x);
            if (shdr->sh_type == SHT_SYMTAB) {
                sym_shdr = shdr;
            }
        }
    }

    if (!sym_shdr) {
        printk("can not locate section for symbol table");
        return NULL;
    }

    {
        printk("Loading sections");
        for (uint32_t x = 0; x < ehdr->e_shentsize * ehdr->e_shnum; x += ehdr->e_shentsize) {
            elf32_shdr *shdr = (elf32_shdr *)((uint32_t)ehdr + ehdr->e_shoff + x);
            if (shdr->sh_type == SHT_NOBITS) {
                shdr->sh_addr = (uint32_t)kmalloc(shdr->sh_size);
                memset(shdr->sh_addr, 0x0, shdr->sh_size);
            } else {
                shdr->sh_addr = (uint32_t)ehdr + shdr->sh_offset;
            }
        }
    }

    int undefined = 0;


    return NULL;
}

void modules_init(multiboot_info_t *mbi) {
    multiboot_module_t *mod;
    int i;

    for (i = 0, mod = (multiboot_module_t *)mbi->mods_addr; i < mbi->mods_count; i++, mod++) {
        if (i == 0) {
            printk("ignore ram disk");
            continue;
        }

        uint32_t mstart = mod->mod_start;
        uint32_t msize = mod->mod_end - mod->mod_start;
        module_load((void *)mstart, msize);
    }
}
