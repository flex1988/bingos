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

syscall|interrupt number
---|---
exit|0
open|2
read|3
write|4
close|5
gettimeofday|6
execve|7
fork|8
getpid|9
sbrk|10
uname|12
openpty|13
seek|14
stat|15
mkpipe|16
dup2|17
getuid|18
setuid|19
reboot|20
readdir|21
chdir|22
getcwd|23
clone|24
sethostname|25
gethostname|26
mkdir|27
shm_obtain|28
shm_release|29
kill|30
signal|31
gettid|32
yield|33
sysfunc|34
sleepabs|35
sleep|36
ioctl|37
access|38
statf|39
chmod|40
umask|41
unlink|42
waitpid|43
pipe|44
mount|45
symlink|46
readlink|47
lstat|48
fswait|49
fswait2|50



