.section .symbols

.extern alloc
.type alloc, @function

.extern alloc_frame
.type alloc_frame, @function

.extern alloc_frames
.type alloc_frames, @function

.extern bitmap_set
.type bitmap_set, @function

.extern bitmap_test
.type bitmap_test, @function

.extern bitmap_unset
.type bitmap_unset, @function

.extern BLACK
.type BLACK, @function

.extern BLUE
.type BLUE, @function

.extern bss
.type bss, @function

.extern _bss
.type _bss, @function

.extern cmp_page_dir
.type cmp_page_dir, @function

.extern code
.type code, @function

.extern _code
.type _code, @function

.extern __code
.type __code, @function

.extern console
.type console, @function

.extern context_switch
.type context_switch, @function

.extern copy_page_physical
.type copy_page_physical, @function

.extern create_heap
.type create_heap, @function

.extern create_tree
.type create_tree, @function

.extern create_tree_node
.type create_tree_node, @function

.extern _current_pd
.type _current_pd, @function

.extern _current_process
.type _current_process, @function

.extern data
.type data, @function

.extern default_comparer
.type default_comparer, @function

.extern destroy_ordered_array
.type destroy_ordered_array, @function

.extern dirent
.type dirent, @function

.extern do_mmap
.type do_mmap, @function

.extern do_munmap
.type do_munmap, @function

.extern draw_char
.type draw_char, @function

.extern draw_pixel
.type draw_pixel, @function

.extern dump_tree_node
.type dump_tree_node, @function

.extern dump_vm_area
.type dump_vm_area, @function

.extern dup_mmap
.type dup_mmap, @function

.extern elf_ehdr_check
.type elf_ehdr_check, @function

.extern elf_load_sections
.type elf_load_sections, @function

.extern enter_userspace
.type enter_userspace, @function

.extern file_headers
.type file_headers, @function

.extern _finished_queue
.type _finished_queue, @function

.extern frame_buffer
.type frame_buffer, @function

.extern frame_buffer_length
.type frame_buffer_length, @function

.extern frame_init
.type frame_init, @function

.extern free
.type free, @function

.extern free_frame
.type free_frame, @function

.extern free_pages
.type free_pages, @function

.extern fs_tree
.type fs_tree, @function

.extern gdt_flush
.type gdt_flush, @function

.extern get_first_frame
.type get_first_frame, @function

.extern get_first_frames
.type get_first_frames, @function

.extern get_page
.type get_page, @function

.extern get_physaddr
.type get_physaddr, @function

.extern getpid
.type getpid, @function

.extern graphic_init
.type graphic_init, @function

.extern GREEN
.type GREEN, @function

.extern hashmap_beigin_with
.type hashmap_beigin_with, @function

.extern hashmap_create
.type hashmap_create, @function

.extern hashmap_delete
.type hashmap_delete, @function

.extern hashmap_free
.type hashmap_free, @function

.extern hashmap_get
.type hashmap_get, @function

.extern hashmap_set
.type hashmap_set, @function

.extern hashmap_string_compare
.type hashmap_string_compare, @function

.extern hashmap_string_dup
.type hashmap_string_dup, @function

.extern hashmap_string_hash
.type hashmap_string_hash, @function

.extern idt_flush
.type idt_flush, @function

.extern inb
.type inb, @function

.extern init_descriptor_tables
.type init_descriptor_tables, @function

.extern _initial_esp
.type _initial_esp, @function

.extern _init_process
.type _init_process, @function

.extern initrd_dev
.type initrd_dev, @function

.extern initrd_header
.type initrd_header, @function

.extern initrd_init
.type initrd_init, @function

.extern initrd_root
.type initrd_root, @function

.extern insert_ordered_array
.type insert_ordered_array, @function

.extern insert_vm
.type insert_vm, @function

.extern _interrupt_handlers
.type _interrupt_handlers, @function

.extern irq0
.type irq0, @function

.extern irq1
.type irq1, @function

.extern irq10
.type irq10, @function

.extern irq11
.type irq11, @function

.extern irq12
.type irq12, @function

.extern irq13
.type irq13, @function

.extern irq14
.type irq14, @function

.extern irq15
.type irq15, @function

.extern irq2
.type irq2, @function

.extern irq3
.type irq3, @function

.extern irq4
.type irq4, @function

.extern irq5
.type irq5, @function

.extern irq6
.type irq6, @function

.extern irq7
.type irq7, @function

.extern irq8
.type irq8, @function

.extern irq9
.type irq9, @function

.extern irq_handler
.type irq_handler, @function

.extern isr0
.type isr0, @function

.extern isr1
.type isr1, @function

.extern isr10
.type isr10, @function

.extern isr11
.type isr11, @function

.extern isr12
.type isr12, @function

.extern isr128
.type isr128, @function

.extern isr13
.type isr13, @function

.extern isr14
.type isr14, @function

.extern isr15
.type isr15, @function

.extern isr16
.type isr16, @function

.extern isr17
.type isr17, @function

.extern isr18
.type isr18, @function

.extern isr19
.type isr19, @function

.extern isr2
.type isr2, @function

.extern isr20
.type isr20, @function

.extern isr21
.type isr21, @function

.extern isr22
.type isr22, @function

.extern isr23
.type isr23, @function

.extern isr24
.type isr24, @function

.extern isr25
.type isr25, @function

.extern isr26
.type isr26, @function

.extern isr27
.type isr27, @function

.extern isr28
.type isr28, @function

.extern isr29
.type isr29, @function

.extern isr3
.type isr3, @function

.extern isr30
.type isr30, @function

.extern isr31
.type isr31, @function

.extern isr4
.type isr4, @function

.extern isr5
.type isr5, @function

.extern isr6
.type isr6, @function

.extern isr7
.type isr7, @function

.extern isr8
.type isr8, @function

.extern isr9
.type isr9, @function

.extern isr_handler
.type isr_handler, @function

.extern kbd_enc_read_buf
.type kbd_enc_read_buf, @function

.extern kbd_init
.type kbd_init, @function

.extern kernel_end
.type kernel_end, @function

.extern _kernel_pd
.type _kernel_pd, @function

.extern kernel_symbols_end
.type kernel_symbols_end, @function

.extern kernel_symbols_start
.type kernel_symbols_start, @function

.extern kfree
.type kfree, @function

.extern kheap
.type kheap, @function

.extern kkybrd_key_to_ascii
.type kkybrd_key_to_ascii, @function

.extern kkybrd_set_leds
.type kkybrd_set_leds, @function

.extern kmain
.type kmain, @function

.extern kmalloc
.type kmalloc, @function

.extern kmalloc_i
.type kmalloc_i, @function

.extern kybrd_ctrl_read_status
.type kybrd_ctrl_read_status, @function

.extern kybrd_enc_send_cmd
.type kybrd_enc_send_cmd, @function

.extern list_create
.type list_create, @function

.extern list_delete
.type list_delete, @function

.extern list_destroy
.type list_destroy, @function

.extern list_pop_front
.type list_pop_front, @function

.extern list_push_back
.type list_push_back, @function

.extern list_push_front
.type list_push_front, @function

.extern list_remove
.type list_remove, @function

.extern local_irq_enable
.type local_irq_enable, @function

.extern lookup_ordered_array
.type lookup_ordered_array, @function

.extern memcmp
.type memcmp, @function

.extern memcpy
.type memcpy, @function

.extern memory_region_deinit
.type memory_region_deinit, @function

.extern memory_region_init
.type memory_region_init, @function

.extern memset
.type memset, @function

.extern mmu_init
.type mmu_init, @function

.extern module_load
.type module_load, @function

.extern modules_init
.type modules_init, @function

.extern move_stack
.type move_stack, @function

.extern _next_pid
.type _next_pid, @function

.extern nroot_nodes
.type nroot_nodes, @function

.extern nsyscalls
.type nsyscalls, @function

.extern outb
.type outb, @function

.extern page_dir_clone
.type page_dir_clone, @function

.extern page_fault
.type page_fault, @function

.extern page_identical_map
.type page_identical_map, @function

.extern page_map
.type page_map, @function

.extern page_unmap
.type page_unmap, @function

.extern panic
.type panic, @function

.extern _pdbr
.type _pdbr, @function

.extern _placement_addr
.type _placement_addr, @function

.extern place_ordered_array
.type place_ordered_array, @function

.extern pre_alloc
.type pre_alloc, @function

.extern printk
.type printk, @function

.extern process_create
.type process_create, @function

.extern process_exit
.type process_exit, @function

.extern process_init
.type process_init, @function

.extern _process_queue
.type _process_queue, @function

.extern read_eip
.type read_eip, @function

.extern RED
.type RED, @function

.extern register_interrupt_handler
.type register_interrupt_handler, @function

.extern relocate_stack
.type relocate_stack, @function

.extern remove_ordered_array
.type remove_ordered_array, @function

.extern root_nodes
.type root_nodes, @function

.extern sched_available
.type sched_available, @function

.extern sched_dequeue
.type sched_dequeue, @function

.extern sched_enqueue
.type sched_enqueue, @function

.extern sched_enqueue_finished
.type sched_enqueue_finished, @function

.extern sched_init
.type sched_init, @function

.extern sched_lookup_finished
.type sched_lookup_finished, @function

.extern set_kernel_stack
.type set_kernel_stack, @function

.extern set_tree_root
.type set_tree_root, @function

.extern setupfonts
.type setupfonts, @function

.extern SOLARIZED_BASE0
.type SOLARIZED_BASE0, @function

.extern SOLARIZED_BASE03
.type SOLARIZED_BASE03, @function

.extern _start
.type _start, @function

.extern strcat
.type strcat, @function

.extern strchr
.type strchr, @function

.extern strcmp
.type strcmp, @function

.extern strcpy
.type strcpy, @function

.extern strlen
.type strlen, @function

.extern strncat
.type strncat, @function

.extern strncmp
.type strncmp, @function

.extern strncpy
.type strncpy, @function

.extern strnlen
.type strnlen, @function

.extern switch_to_next
.type switch_to_next, @function

.extern switch_to_user_mode
.type switch_to_user_mode, @function

.extern sys_brk
.type sys_brk, @function

.extern syscalls_init
.type syscalls_init, @function

.extern sys_close
.type sys_close, @function

.extern sys_exec
.type sys_exec, @function

.extern sys_execve
.type sys_execve, @function

.extern sys_exit
.type sys_exit, @function

.extern sys_fork
.type sys_fork, @function

.extern sys_getpid
.type sys_getpid, @function

.extern sys_gettimeofday
.type sys_gettimeofday, @function

.extern sys_open
.type sys_open, @function

.extern sys_println
.type sys_println, @function

.extern sys_read
.type sys_read, @function

.extern sys_waitpid
.type sys_waitpid, @function

.extern sys_write
.type sys_write, @function

.extern table_clone
.type table_clone, @function

.extern timer_init
.type timer_init, @function

.extern traverse_tree
.type traverse_tree, @function

.extern travse_dir
.type travse_dir, @function

.extern tree_node_insert
.type tree_node_insert, @function

.extern tree_node_search
.type tree_node_search, @function

.extern tree_search
.type tree_search, @function

.extern tss_entry
.type tss_entry, @function

.extern tss_flush
.type tss_flush, @function

.extern tty_init
.type tty_init, @function

.extern vesa_init
.type vesa_init, @function

.extern vfs_close
.type vfs_close, @function

.extern vfs_finddir
.type vfs_finddir, @function

.extern vfs_get_mount_point
.type vfs_get_mount_point, @function

.extern vfs_init
.type vfs_init, @function

.extern vfs_lookup
.type vfs_lookup, @function

.extern vfs_mount
.type vfs_mount, @function

.extern vfs_open
.type vfs_open, @function

.extern vfs_read
.type vfs_read, @function

.extern vfs_readdir
.type vfs_readdir, @function

.extern vfs_root
.type vfs_root, @function

.extern vfs_write
.type vfs_write, @function

.extern vga_init
.type vga_init, @function

.extern vsprintf
.type vsprintf, @function

.global kernel_symbols_start
kernel_symbols_start:

.long alloc
.asciz "alloc"

.long alloc_frame
.asciz "alloc_frame"

.long alloc_frames
.asciz "alloc_frames"

.long bitmap_set
.asciz "bitmap_set"

.long bitmap_test
.asciz "bitmap_test"

.long bitmap_unset
.asciz "bitmap_unset"

.long BLACK
.asciz "BLACK"

.long BLUE
.asciz "BLUE"

.long bss
.asciz "bss"

.long _bss
.asciz "_bss"

.long cmp_page_dir
.asciz "cmp_page_dir"

.long code
.asciz "code"

.long _code
.asciz "_code"

.long __code
.asciz "__code"

.long console
.asciz "console"

.long context_switch
.asciz "context_switch"

.long copy_page_physical
.asciz "copy_page_physical"

.long create_heap
.asciz "create_heap"

.long create_tree
.asciz "create_tree"

.long create_tree_node
.asciz "create_tree_node"

.long _current_pd
.asciz "_current_pd"

.long _current_process
.asciz "_current_process"

.long data
.asciz "data"

.long default_comparer
.asciz "default_comparer"

.long destroy_ordered_array
.asciz "destroy_ordered_array"

.long dirent
.asciz "dirent"

.long do_mmap
.asciz "do_mmap"

.long do_munmap
.asciz "do_munmap"

.long draw_char
.asciz "draw_char"

.long draw_pixel
.asciz "draw_pixel"

.long dump_tree_node
.asciz "dump_tree_node"

.long dump_vm_area
.asciz "dump_vm_area"

.long dup_mmap
.asciz "dup_mmap"

.long elf_ehdr_check
.asciz "elf_ehdr_check"

.long elf_load_sections
.asciz "elf_load_sections"

.long enter_userspace
.asciz "enter_userspace"

.long file_headers
.asciz "file_headers"

.long _finished_queue
.asciz "_finished_queue"

.long frame_buffer
.asciz "frame_buffer"

.long frame_buffer_length
.asciz "frame_buffer_length"

.long frame_init
.asciz "frame_init"

.long free
.asciz "free"

.long free_frame
.asciz "free_frame"

.long free_pages
.asciz "free_pages"

.long fs_tree
.asciz "fs_tree"

.long gdt_flush
.asciz "gdt_flush"

.long get_first_frame
.asciz "get_first_frame"

.long get_first_frames
.asciz "get_first_frames"

.long get_page
.asciz "get_page"

.long get_physaddr
.asciz "get_physaddr"

.long getpid
.asciz "getpid"

.long graphic_init
.asciz "graphic_init"

.long GREEN
.asciz "GREEN"

.long hashmap_beigin_with
.asciz "hashmap_beigin_with"

.long hashmap_create
.asciz "hashmap_create"

.long hashmap_delete
.asciz "hashmap_delete"

.long hashmap_free
.asciz "hashmap_free"

.long hashmap_get
.asciz "hashmap_get"

.long hashmap_set
.asciz "hashmap_set"

.long hashmap_string_compare
.asciz "hashmap_string_compare"

.long hashmap_string_dup
.asciz "hashmap_string_dup"

.long hashmap_string_hash
.asciz "hashmap_string_hash"

.long idt_flush
.asciz "idt_flush"

.long inb
.asciz "inb"

.long init_descriptor_tables
.asciz "init_descriptor_tables"

.long _initial_esp
.asciz "_initial_esp"

.long _init_process
.asciz "_init_process"

.long initrd_dev
.asciz "initrd_dev"

.long initrd_header
.asciz "initrd_header"

.long initrd_init
.asciz "initrd_init"

.long initrd_root
.asciz "initrd_root"

.long insert_ordered_array
.asciz "insert_ordered_array"

.long insert_vm
.asciz "insert_vm"

.long _interrupt_handlers
.asciz "_interrupt_handlers"

.long irq0
.asciz "irq0"

.long irq1
.asciz "irq1"

.long irq10
.asciz "irq10"

.long irq11
.asciz "irq11"

.long irq12
.asciz "irq12"

.long irq13
.asciz "irq13"

.long irq14
.asciz "irq14"

.long irq15
.asciz "irq15"

.long irq2
.asciz "irq2"

.long irq3
.asciz "irq3"

.long irq4
.asciz "irq4"

.long irq5
.asciz "irq5"

.long irq6
.asciz "irq6"

.long irq7
.asciz "irq7"

.long irq8
.asciz "irq8"

.long irq9
.asciz "irq9"

.long irq_handler
.asciz "irq_handler"

.long isr0
.asciz "isr0"

.long isr1
.asciz "isr1"

.long isr10
.asciz "isr10"

.long isr11
.asciz "isr11"

.long isr12
.asciz "isr12"

.long isr128
.asciz "isr128"

.long isr13
.asciz "isr13"

.long isr14
.asciz "isr14"

.long isr15
.asciz "isr15"

.long isr16
.asciz "isr16"

.long isr17
.asciz "isr17"

.long isr18
.asciz "isr18"

.long isr19
.asciz "isr19"

.long isr2
.asciz "isr2"

.long isr20
.asciz "isr20"

.long isr21
.asciz "isr21"

.long isr22
.asciz "isr22"

.long isr23
.asciz "isr23"

.long isr24
.asciz "isr24"

.long isr25
.asciz "isr25"

.long isr26
.asciz "isr26"

.long isr27
.asciz "isr27"

.long isr28
.asciz "isr28"

.long isr29
.asciz "isr29"

.long isr3
.asciz "isr3"

.long isr30
.asciz "isr30"

.long isr31
.asciz "isr31"

.long isr4
.asciz "isr4"

.long isr5
.asciz "isr5"

.long isr6
.asciz "isr6"

.long isr7
.asciz "isr7"

.long isr8
.asciz "isr8"

.long isr9
.asciz "isr9"

.long isr_handler
.asciz "isr_handler"

.long kbd_enc_read_buf
.asciz "kbd_enc_read_buf"

.long kbd_init
.asciz "kbd_init"

.long kernel_end
.asciz "kernel_end"

.long _kernel_pd
.asciz "_kernel_pd"

.long kernel_symbols_end
.asciz "kernel_symbols_end"

.long kernel_symbols_start
.asciz "kernel_symbols_start"

.long kfree
.asciz "kfree"

.long kheap
.asciz "kheap"

.long kkybrd_key_to_ascii
.asciz "kkybrd_key_to_ascii"

.long kkybrd_set_leds
.asciz "kkybrd_set_leds"

.long kmain
.asciz "kmain"

.long kmalloc
.asciz "kmalloc"

.long kmalloc_i
.asciz "kmalloc_i"

.long kybrd_ctrl_read_status
.asciz "kybrd_ctrl_read_status"

.long kybrd_enc_send_cmd
.asciz "kybrd_enc_send_cmd"

.long list_create
.asciz "list_create"

.long list_delete
.asciz "list_delete"

.long list_destroy
.asciz "list_destroy"

.long list_pop_front
.asciz "list_pop_front"

.long list_push_back
.asciz "list_push_back"

.long list_push_front
.asciz "list_push_front"

.long list_remove
.asciz "list_remove"

.long local_irq_enable
.asciz "local_irq_enable"

.long lookup_ordered_array
.asciz "lookup_ordered_array"

.long memcmp
.asciz "memcmp"

.long memcpy
.asciz "memcpy"

.long memory_region_deinit
.asciz "memory_region_deinit"

.long memory_region_init
.asciz "memory_region_init"

.long memset
.asciz "memset"

.long mmu_init
.asciz "mmu_init"

.long module_load
.asciz "module_load"

.long modules_init
.asciz "modules_init"

.long move_stack
.asciz "move_stack"

.long _next_pid
.asciz "_next_pid"

.long nroot_nodes
.asciz "nroot_nodes"

.long nsyscalls
.asciz "nsyscalls"

.long outb
.asciz "outb"

.long page_dir_clone
.asciz "page_dir_clone"

.long page_fault
.asciz "page_fault"

.long page_identical_map
.asciz "page_identical_map"

.long page_map
.asciz "page_map"

.long page_unmap
.asciz "page_unmap"

.long panic
.asciz "panic"

.long _pdbr
.asciz "_pdbr"

.long _placement_addr
.asciz "_placement_addr"

.long place_ordered_array
.asciz "place_ordered_array"

.long pre_alloc
.asciz "pre_alloc"

.long printk
.asciz "printk"

.long process_create
.asciz "process_create"

.long process_exit
.asciz "process_exit"

.long process_init
.asciz "process_init"

.long _process_queue
.asciz "_process_queue"

.long read_eip
.asciz "read_eip"

.long RED
.asciz "RED"

.long register_interrupt_handler
.asciz "register_interrupt_handler"

.long relocate_stack
.asciz "relocate_stack"

.long remove_ordered_array
.asciz "remove_ordered_array"

.long root_nodes
.asciz "root_nodes"

.long sched_available
.asciz "sched_available"

.long sched_dequeue
.asciz "sched_dequeue"

.long sched_enqueue
.asciz "sched_enqueue"

.long sched_enqueue_finished
.asciz "sched_enqueue_finished"

.long sched_init
.asciz "sched_init"

.long sched_lookup_finished
.asciz "sched_lookup_finished"

.long set_kernel_stack
.asciz "set_kernel_stack"

.long set_tree_root
.asciz "set_tree_root"

.long setupfonts
.asciz "setupfonts"

.long SOLARIZED_BASE0
.asciz "SOLARIZED_BASE0"

.long SOLARIZED_BASE03
.asciz "SOLARIZED_BASE03"

.long _start
.asciz "_start"

.long strcat
.asciz "strcat"

.long strchr
.asciz "strchr"

.long strcmp
.asciz "strcmp"

.long strcpy
.asciz "strcpy"

.long strlen
.asciz "strlen"

.long strncat
.asciz "strncat"

.long strncmp
.asciz "strncmp"

.long strncpy
.asciz "strncpy"

.long strnlen
.asciz "strnlen"

.long switch_to_next
.asciz "switch_to_next"

.long switch_to_user_mode
.asciz "switch_to_user_mode"

.long sys_brk
.asciz "sys_brk"

.long syscalls_init
.asciz "syscalls_init"

.long sys_close
.asciz "sys_close"

.long sys_exec
.asciz "sys_exec"

.long sys_execve
.asciz "sys_execve"

.long sys_exit
.asciz "sys_exit"

.long sys_fork
.asciz "sys_fork"

.long sys_getpid
.asciz "sys_getpid"

.long sys_gettimeofday
.asciz "sys_gettimeofday"

.long sys_open
.asciz "sys_open"

.long sys_println
.asciz "sys_println"

.long sys_read
.asciz "sys_read"

.long sys_waitpid
.asciz "sys_waitpid"

.long sys_write
.asciz "sys_write"

.long table_clone
.asciz "table_clone"

.long timer_init
.asciz "timer_init"

.long traverse_tree
.asciz "traverse_tree"

.long travse_dir
.asciz "travse_dir"

.long tree_node_insert
.asciz "tree_node_insert"

.long tree_node_search
.asciz "tree_node_search"

.long tree_search
.asciz "tree_search"

.long tss_entry
.asciz "tss_entry"

.long tss_flush
.asciz "tss_flush"

.long tty_init
.asciz "tty_init"

.long vesa_init
.asciz "vesa_init"

.long vfs_close
.asciz "vfs_close"

.long vfs_finddir
.asciz "vfs_finddir"

.long vfs_get_mount_point
.asciz "vfs_get_mount_point"

.long vfs_init
.asciz "vfs_init"

.long vfs_lookup
.asciz "vfs_lookup"

.long vfs_mount
.asciz "vfs_mount"

.long vfs_open
.asciz "vfs_open"

.long vfs_read
.asciz "vfs_read"

.long vfs_readdir
.asciz "vfs_readdir"

.long vfs_root
.asciz "vfs_root"

.long vfs_write
.asciz "vfs_write"

.long vga_init
.asciz "vga_init"

.long vsprintf
.asciz "vsprintf"

.global kernel_symbols_end
kernel_symbols_end:
