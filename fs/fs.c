#include "fs/fs.h"
#include "kernel.h"
#include "lib/hashmap.h"
#include "lib/tree.h"

#include <errno.h>
#include <string.h>

vfs_node_t* vfs_root = NULL;
tree_t* vfs_tree = NULL;
hashmap_t* vfs_type_mounts = NULL;

static vfs_node_t* vfs_lookup_internal(vfs_node_t* n, char* path, int depth) {
    char* dir = NULL;
    vfs_node_t* ret = NULL;

    if (!path[0])
        return n;

    if (depth == 0)
        return vfs_finddir(n, path);

    while (depth-- > 0) {
        dir = path;

        path = path + strlen(path) + 1;

        ret = vfs_finddir(n, dir);
    }

    return ret;
}

uint32_t vfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node->read)
        return node->read(node, offset, size, buffer);
    else
        return 0;
}

uint32_t vfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node->write)
        return node->write(node, offset, size, buffer);
    return 0;
}

void vfs_open(vfs_node_t* node, uint8_t flags) {
    if (node->open)
        return node->open(node);
}

int vfs_close(vfs_node_t* node) {
    if (node->close)
        return node->close(node);
}

dirent_t* vfs_readdir(vfs_node_t* node, uint32_t index) {
    if ((node->flags & 0x7) == VFS_DIRECTORY && node->readdir)
        return node->readdir(node, index);
    else
        return 0;
}

vfs_node_t* vfs_finddir(vfs_node_t* node, char* name) {
    if ((node->flags & 0x7) == VFS_DIRECTORY && node->finddir) {
        return node->finddir(node, name);
    } else
        return 0;
}

vfs_node_t* vfs_clone(vfs_node_t* source) {
    if (!source)
        return NULL;

    if (source->refs >= 0) {
        source->refs++;
    }

    return source;
}

int vfs_mkdir(vfs_node_t* dir, const char* name, int len, int mode) {
    printk("vfs_mkdir 0x%x 0x%x",dir,dir->mkdir);
    if (dir && dir->mkdir)
        return dir->mkdir(dir, name, len, mode);
    return -ENOENT;
}

// find file mount point
vfs_node_t* vfs_get_mount_point(char* path, uint32_t depth, char** mount_path, uint32_t* mount_depth) {
    size_t d;

    for (d = 0; d < depth; d++) {
        path += strlen(path) + 1;
    }

    vfs_node_t* last = vfs_root;
    tree_node_t* node = vfs_tree->root;

    char* p = *mount_path;
    *mount_depth = 1;

    while (1) {
        if (p >= path) {
            break;
        }

        int found = 0;
        for (int i = 0; i < node->length; i++) {
            tree_node_t* child = node->children[i];

            vfs_entry_t* entry = (vfs_entry_t*)child->data;

            if (!strcmp(entry->name, p)) {
                printk("%s %s",entry->name,p);
                found = 1;
                node = child;
                p = p + strlen(p) + 1;

                if (entry->file) {
                    last = entry->file;
                    *mount_path = p;
                }

                break;
            }
        }

        if (!found) {
            break;
        }

        (*mount_depth)++;
    }

    return last;
}

vfs_node_t* vfs_fetch_device(char* path) {
    size_t plen = strlen(path);
    char* pdup = kmalloc(plen + 1);
    memcpy(pdup, path, plen + 1);
    int found;

    char* p = pdup;
    while (p < pdup + plen) {
        if (*p == '/')
            *p = '\0';
        p++;
    }
    pdup[plen] = '\0';

    tree_node_t* node = vfs_tree->root;

    p = pdup;
    p++;
    while (1) {
        if (p > pdup + plen) {
            break;
        }

        found = 0;
        for (int i = 0; i < node->length; i++) {
            tree_node_t* child = node->children[i];
            vfs_entry_t* entry = (vfs_entry_t*)child->data;

            if (!strcmp(entry->name, p)) {
                node = child;
                found = 1;
                break;
            }
        }

        // create vfs_node if node not exits
        if (!found) {
            break;
        }

        p += strlen(p) + 1;
    }

    kfree(pdup);

    if (!found)
        return NULL;

    vfs_entry_t* entry = (vfs_entry_t*)node->data;
    return entry->file;
}

int vfs_lookup_child(vfs_node_t* parent, const char* name, int len, vfs_node_t** ret) {
    vfs_node_t* n;

    char tmp[256];
    memcpy(tmp, name, len);
    tmp[len] = '\0';

    n = vfs_finddir(parent, tmp);

    if (!n)
        return -ENOENT;

    *ret = n;
    kfree(n);

    return 0;
}

vfs_node_t* vfs_lookup(const char* path, int type) {
    char* dup = NULL;
    vfs_node_t* ret = NULL;
    vfs_node_t* c = NULL;

    // support absolute path only
    ASSERT(path[0] == '/');

    dup = kmalloc(strlen(path) + 1);
    memcpy(dup, path, strlen(path) + 1);

    size_t path_size = strlen(path);

    if (path_size == 1) {
        kfree(dup);
        return vfs_root;
    }

    char* p = dup;
    uint32_t pdepth = 0;

    // split path
    while (p < dup + path_size) {
        if (*p == '/') {
            *p = '\0';
            pdepth++;
        }
        p++;
    }
    p = dup + 1;

    char* mount_path = p;
    uint32_t mount_depth = 0;

    vfs_node_t* node = vfs_get_mount_point(p, pdepth, &mount_path, &mount_depth);

    if (mount_depth <= pdepth) {
        ret = vfs_lookup_internal(node, mount_path, pdepth - mount_depth);
    } else {
        ret = node;
    }

    kfree(dup);

    return ret;
}

void* vfs_mount(char* path, vfs_node_t* lroot) {
    if (!vfs_tree) {
        printk("vfs mount tree has not been initialized");
        return NULL;
    }

    if (!path || path[0] != '/') {
        printk("mount path must be absolute path");
        return NULL;
    }

    // may be need spin lock
    tree_node_t* ret = NULL;

    int path_size = strlen(path);
    char* buf = kmalloc(path_size + 1);
    memcpy(buf, path, path_size + 1);

    char* i = buf;
    while (i < buf + path_size) {
        if (*i == '/') {
            *i = '\0';
        }
        i++;
    }

    buf[path_size] = '\0';
    i = buf + 1;

    tree_node_t* root_node = vfs_tree->root;

    if (*i == '\0') {
        // mount root
        vfs_entry_t* root = (vfs_entry_t*)root_node->data;
        if (root->file) {
            printk("Path %s already mounted", path);
        }
        root->file = lroot;
        vfs_root = lroot;
        ret = root_node;
    } else {
        tree_node_t* node = root_node;
        char* p = i;

        while (1) {
            if (p >= buf + path_size)
                break;

            int found = 0;

            for (int j = 0; j < node->length; j++) {
                tree_node_t* child = node->children[j];

                if (!strcmp(child->name, p)) {
                    found = 1;
                    node = child;
                    ret = child;
                    break;
                }
            }

            if (!found) {
                vfs_entry_t* entry = kmalloc(sizeof(vfs_entry_t));
                memcpy(entry->name, p, strlen(p) + 1);

                entry->file = NULL;
                entry->device = NULL;
                entry->type = NULL;

                node = tree_node_insert(node, entry->name, strlen(p) + 1, entry);
                memcpy(node->name, entry->name, strlen(entry->name) + 1);
            }

            p = p + strlen(p) + 1;
        }

        vfs_entry_t* entry = (vfs_entry_t*)node->data;
        if (entry->file) {
            printk("Path %s already mounted", path);
        }
        entry->file = lroot;
        ret = node;
    }

    kfree(buf);

    return ret;
}

int vfs_register(char* name, vfs_mount_callback callback) {
    if (hashmap_get(vfs_type_mounts, name))
        return 1;
    hashmap_set(vfs_type_mounts, name, (void*)callback);
    return 0;
}

int vfs_mount_type(char* type, char* arg, char* mount_point) {
    vfs_mount_callback mount = (vfs_mount_callback)(uint32_t)hashmap_get(vfs_type_mounts, type);
    if (!mount) {
        printk("unknown filesystem : %s", type);
        return -ENODEV;
    }

    vfs_node_t* n = mount(arg, mount_point);
    printk("ext2 0x%x", n);
    if (!n)
        return -EINVAL;

    tree_node_t* node = vfs_mount(mount_point, n);
    if (node && node->data) {
        vfs_entry_t* entry = (vfs_entry_t*)node->data;
        /*entry->type = strdup(type);*/
        /*entry->device = strdup(arg);*/
    }

    return 0;
}

void vfs_init() {
    vfs_tree = create_tree();
    vfs_entry_t* root = kmalloc(sizeof(vfs_entry_t));

    memcpy(root->name, "/", strlen("/") + 1);

    root->file = NULL;
    root->type = NULL;
    root->device = NULL;

    tree_node_t* n = kmalloc(sizeof(tree_node_t));
    n->data = (void*)root;
    n->length = 0;

    memcpy(n->name, root->name, strlen(root->name) + 1);
    memset(n->children, 0, MAX_TREE_CHILDREN * sizeof(tree_node_t*));

    set_tree_root(vfs_tree, n);
}
