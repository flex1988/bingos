#ifndef __INITRD_H__
#define __INITRD_H__

#include "types.h"
#include "fs/fs.h"
#include "multiboot.h"

typedef struct { uint32_t nfiles; } initrd_header_t;

typedef struct {
    uint8_t magic;
    int8_t name[64];
    uint32_t offset;
    uint32_t length;
} initrd_file_header_t;

void ramdisk_init(multiboot_info_t *boot_info);
#endif
