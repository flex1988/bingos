#include <stdio.h>
#include <syscall.h>
#include <types.h>
#include <dirent.h>

int main(int argc, char **argv) {
    if(!argv[1]) {
        println("Invalid arguments.");
        return 0;
    }

    char *target = argv[1];
    stat_t buf;
    int ret = stat(target,&buf);
    if(ret < 0) {
        println("stat error.");
        return 0;
    }
    
    if(buf.st_mode == S_DIRECTORY) {
        dirent_t *entry;
        DIR *dir = opendir(target);

        while(entry = readdir(dir)) {
            println("%s",entry->d_name);
        }

        closedir(dir);
    } else if(buf.st_mode == S_FILE) {
        println("%s",target);
    }

    return 0;
}
