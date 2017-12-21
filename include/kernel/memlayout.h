#ifndef __MEMLAYOUT_H__
#define __MEMLAYOUT_H__

/* Memory layout definition
 * +------------+
 * | 0x00100000 | Kernel multiboot header
 * +------------+
 * | 0x00x00000 | some kernel stucts before mmu_init
 * +------------+
 * | 0x20000000 | User mode image loaded address
 * +------------+
 * | 0x30000000 | User stack: 0x30000000~0x30010000
 * +------------+
 * | 0x30010000 | User stack heap bottom
 * +------------+
 * | 0xc0000000 | Kernel heap bottom
 * +------------+
 * | 0xdfffe000 | Init process kernel stack: 0xdfffe000~0xe0000000
 * +------------+
 * | 0xe0000000 | 
 * +------------+
 */

#define KSTACK_SIZE 0x2000

#define PROCESS_IMAGE 0x20000000

#define USTACK_SIZE 0x10000
#define USTACK_BOTTOM 0x30000000

#define UHEAP_START 0x30100000
#define UHEAP_INITIAL_SIZE 0x200000

#define KHEAP_START 0xc0000000
#define KHEAP_INITIAL_SIZE 0x100000
#define HEAP_MIN_SIZE 0x70000

#define KINIT_STACK_BOTTOM 0xe0000000
#define KINIT_STACK_SIZE 0x2000

#endif
