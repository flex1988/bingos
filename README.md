### kernel x86

### Build && Run

1. http://wiki.osdev.org/GCC_Cross-Compiler
2. sh build.sh
3. qemu-system-x86_64 -cdrom build/os-x86_64.iso

### GDB Debug

    - gdb build/kernel-x86_64.bin
    - set arch i386:x86-64:intel
    - b kmain
    - target remote :1234

### Syscall Table

syscall|interrupt number|comment|args
---|---|---|---
exit|0|exit user program|int sys_exit(int ret)
println|1|printk msg|int sys_println(char *)
open|2|open for reading or writing|int open(const char *path, int oflag, ... /*mode_t mode */)
read|3|read input|int read(int d, void * buf, size_t nbytes);
write|4|write output|ssize_t write(int d, const void * buf, size_t nbytes);
close|5|close a file descriptor
gettimeofday|6|get/set date and time
execve|7|transforms the calling process into a new process
fork|8|copy calling process
getpid|9|get parent or calling process identification
sbrk|10|change data segment size
lseek|11|reposition read/write file offset
lstat|12|get file status
dup2|13|duplicate an open file descriptor
readdir|14|directory operations
chdir|15|directory operations
sethostname|16|set the name of the machine
gethostname|17|get the name of the machine
mkdir|18|make a directory file
shm_open|19|open a shared memory object
shm_unlink|20|unlink a shared memory object
ioctl|21|control device
access|22|check access permissions of a file or pathname
fstatfs|23|get file system statistics
fsync|24|synchronize a file's in-memory state with that on the physical medium
ftruncate|25|truncate a file to a specified length
chmod|26|change mode of file
umask|27|set file creation mode mask
link|28|make a hard file link
unlink|29|remove a directory entry
pipe|30|create descriptor a pair for interprocess communication
mount|31|mount or unmount a filesystem
symlink|32|make a symbolic link to a file
readlink|33|read value of a symbolic link

### DOCS

- [VESA Tutorial](http://wiki.osdev.org/User:Omarrx024/VESA_Tutorial)
- [QEMU Debug](http://wiki.osdev.org/QEMU_and_GDB_in_long_mode)
- [Multiboot](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
- [Drawing_In_Protected](http://wiki.osdev.org/Drawing_In_Protected_Mode#Drawing_Text)
