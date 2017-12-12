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
waitpid|10|
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

