#include "kernel.h"
#include "module.h"

int ext2_init(void) { return 0; }

int ext2_deinit(void) { return 0; }

MODULE_DEFINE(ext2, ext2_init, ext2_deinit);
