#include "module.h"
#include "kernel.h"
#include "kernel/elf.h"
#include "lib/hashmap.h"

#define SYMBOLTABLE_HASHMAP_SIZE 200
#define MODULE_HASHMAP_SIZE 10

#define MODULE_RAMDISK_INDEX 0
#define MODULE_ATA_INDEX 1
#define MODULE_EXT2_INDEX 2

static hashmap_t *symboltable = NULL;
static hashmap_t *modules = NULL;
extern hashmap_t *vfs_type_mounts;

extern char kernel_symbols_start[];
extern char kernel_symbols_end[];

typedef struct {
    uint32_t addr;
    char name[];
} kernel_symbol_t;

extern ptr_t _placement_addr;

void *module_load(void *blob, size_t length, char *mod_name) {
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
    hashmap_t *local_symbols = hashmap_create(10, HASHMAP_STRING);
    {
        elf32_sym *table = (elf32_sym *)((uint32_t)ehdr + sym_shdr->sh_offset);
        while ((uint32_t)table - ((uint32_t)ehdr + sym_shdr->sh_offset) < sym_shdr->sh_size) {
            if (table->st_name) {
                if (ELF32_ST_BIND(table->st_info) == STB_GLOBAL) {
                    char *name = (char *)((uint32_t)symstrtab + table->st_name);
                    if (table->st_shndx == 0) {
                        if (!hashmap_get(symboltable, name)) {
                            printk("unresolved symbol %s", name);
                            undefined = 1;
                        }
                    } else {
                        elf32_shdr *s = NULL;
                        {
                            int i = 0;
                            int set = 0;
                            for (uint32_t x = 0; x < ehdr->e_shentsize * ehdr->e_shnum; x += ehdr->e_shentsize) {
                                elf32_shdr *shdr = (elf32_shdr *)((uint32_t)ehdr + ehdr->e_shoff + x);
                                if (i == table->st_shndx) {
                                    set = 1;
                                    s = shdr;
                                    break;
                                }
                                i++;
                            }

                            if (!set && table->st_shndx == 65522) {
                                if (!hashmap_get(symboltable, name)) {
                                    void * final = kmalloc(table->st_value);
                                    hashmap_set(symboltable, name, (void *)final);
                                    hashmap_set(local_symbols, name, (void *)final);
                                }
                            }
                        }

                        if (s) {
                            uint32_t final = s->sh_addr + table->st_value;
                            hashmap_set(symboltable, name, (void *)final);
                            hashmap_set(local_symbols, name, (void *)final);
                        }
                    }
                }
            }
            table++;
        }
    }

    if (undefined) {
        printk("module invalid");
        return NULL;
    }

    {
        for (uint32_t x = 0; x < ehdr->e_shentsize * ehdr->e_shnum; x += ehdr->e_shentsize) {
            elf32_shdr *shdr = (elf32_shdr *)((uint32_t)ehdr + ehdr->e_shoff + x);
            if (shdr->sh_type == SHT_REL) {
                elf32_rel *section_rel = (void *)(shdr->sh_addr);
                elf32_rel *table = section_rel;
                elf32_sym *symtable = (elf32_sym *)(sym_shdr->sh_addr);

                while ((uint32_t)table - shdr->sh_addr < shdr->sh_size) {
                    elf32_sym *sym = &symtable[ELF32_R_SYM(table->r_info)];
                    elf32_shdr *rs = (elf32_shdr *)((uint32_t)ehdr + ehdr->e_shoff + shdr->sh_info * ehdr->e_shentsize);

                    uint32_t addend = 0;
                    uint32_t place = 0;
                    uint32_t symbol = 0;
                    uint32_t *ptr = NULL;

                    if (ELF32_ST_TYPE(sym->st_info) == STT_SECTION) {
                        elf32_shdr *s = (elf32_shdr *)((uint32_t)ehdr + ehdr->e_shoff + sym->st_shndx * ehdr->e_shentsize);
                        ptr = (uint32_t *)(table->r_offset + rs->sh_addr);
                        addend = *ptr;
                        place = (uint32_t)ptr;
                        symbol = s->sh_addr;
                    } else {
                        char *name = (char *)((uint32_t)symstrtab + sym->st_name);
                        ptr = (uint32_t *)(table->r_offset + rs->sh_addr);
                        addend = *ptr;
                        place = (uint32_t)ptr;

                        if (!hashmap_get(symboltable, name)) {
                            printk("missing symbol %s", name);
                        }

                        symbol = (uint32_t)hashmap_get(symboltable, name);
                    }

                    switch (ELF32_R_TYPE(table->r_info)) {
                        case 1:
                            *ptr = addend + symbol;
                            break;
                        case 2:
                            *ptr = addend + symbol - place;
                            break;
                        default:
                            return NULL;
                    }

                    table++;
                }
            }
        }
    }

    module_t *mod = NULL;
    char name[32];
    sprintf(name, "module_info_%s", mod_name);
    mod = hashmap_get(local_symbols, name);
    mod->init();

    module_info_t *mod_info = kmalloc(sizeof(module_info_t));
    mod_info->mod = mod;
    mod_info->binary = ehdr;
    mod_info->symbols = local_symbols;
    mod_info->end = (uint32_t)ehdr + length;
    mod_info->deps = NULL;
    mod_info->deps_length = 0;

    hashmap_set(modules, mod->name, (void *)mod_info);

    return NULL;
}

void modules_init(multiboot_info_t *mbi) {
    _placement_addr = ((multiboot_module_t *)(mbi->mods_addr) + mbi->mods_count - 1)->mod_end + 0x1000;

    symboltable = hashmap_create(SYMBOLTABLE_HASHMAP_SIZE, HASHMAP_STRING);
    modules = hashmap_create(MODULE_HASHMAP_SIZE, HASHMAP_STRING);
    vfs_type_mounts = hashmap_create(10, HASHMAP_STRING);

    kernel_symbol_t *k = (kernel_symbol_t *)kernel_symbols_start;

    while ((uint32_t)k < (uint32_t)kernel_symbols_end) {
        hashmap_set(symboltable, k->name, (void *)k->addr);
        k = (kernel_symbol_t *)((uint32_t)k + sizeof(kernel_symbol_t) + strlen(k->name) + 1);
    }

    hashmap_set(symboltable, "kernel_symbols_start", kernel_symbols_start);
    hashmap_set(symboltable, "kernel_symbols_end", kernel_symbols_end);

    multiboot_module_t *mod;
    int i;

    for (i = 0, mod = (multiboot_module_t *)mbi->mods_addr; i < mbi->mods_count; i++, mod++) {
        if (i == MODULE_RAMDISK_INDEX) {
            printk("ignore ram disk");
            continue;
        }

        uint32_t mstart = mod->mod_start;
        uint32_t msize = mod->mod_end - mod->mod_start;

        module_load((void *)mstart, msize, mod->cmdline);
    }
}
