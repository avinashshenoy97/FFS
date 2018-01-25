#include "ffs_operations.h"
#include "tree.h"


int ffs_getattr(const char *path, struct stat *s) {
    printf("ffs_getattr called on path : %s\n", path);

    if(!node_exists(path)) {
        return -ENOENT;
    }

    memset(s, 0, sizeof(struct stat));

    s->st_mode = S_IFDIR | 0755;
    s->st_nlink = 2;
    s->st_size = 1024;

    time(&(s->st_atime));
    
    return 0;
}

int ffs_mkdir(const char *path, mode_t m) {
    printf("Add FS tree node at path : %s\n", path);
    add_fs_tree_node(path, 2);
    return 0;
}

int ffs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    printf("ffs_readdir called on path : %s\n", path);

    fs_tree_node *curr = NULL;

    filler(buffer, ".", NULL, 0);
    filler(buffer, "..", NULL, 0);

    curr = node_exists(path);       //check if it exists

    if(strcmp(path, "/")) {             //if its not root
        if(!curr) {
            return -ENOENT;
        }

        //curr = curr->parent;            //get link to its parent node
    }

    printf("Path : %s : found to exist with %d children\n", path, curr->len);

    int i;
    for(i = 0 ; i < curr->len ; i++)
        filler(buffer, curr->children[i].name, NULL, 0);

    return 0;
}