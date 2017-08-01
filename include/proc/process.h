#ifndef __PROCESS_H__
#define __PROCESS_H__

#define USTACK_BOTTOM 0x30000000

typedef struct {
    int ref;
    uint32_t id;
    uint32_t uid;
    uint32_t gid;

    int flags;
    int8_t priority;
    char *name;
    int status;
} process_t;

//process_t process_create();

#endif
