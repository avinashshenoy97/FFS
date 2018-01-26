#include "ffs_operations.h"
#include "tree.h"

// Error logging for THIS MODULE, helps differentiate from logging of other modules
// Prints errors and logging info to STDOUT
// Passes format strings and args to vprintf, basically a wrapper for printf
static void error_log(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    printf("\n");
    printf("FfS OPS : ");
    vprintf(fmt, args);
    printf("\n");

    va_end(args);
}

int ffs_getattr(const char *path, struct stat *s) {
    error_log("ffs_getattr called on path : %s\n", path);

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
    error_log("Add FS tree node at path : %s\n", path);
    add_fs_tree_node(path, 2);
    return 0;
}

int ffs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    error_log("ffs_readdir called on path : %s\n", path);

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

    error_log("Path : %s : found to exist with %d children\n", path, curr->len);

    int i;
    for(i = 0 ; i < curr->len ; i++)
        filler(buffer, curr->children[i]->name, NULL, 0);

    return 0;
}

int ffs_rmdir(const char *path) {
    // OS checks if path exists using getattr, no need to check explicitly
    // Just forward responsibility to tree.c function

    return remove_fs_tree_node(path);
}