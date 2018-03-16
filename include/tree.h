#ifndef FFS_TREE_H
#define FFS_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "disk.h"
#include "bitmap.h"

#define DEF_DIR_PERM (0775)
#define DEF_FILE_PERM (0664)

// fields = type + name + len + uid + gid + perms + nlinks + data_size + atim + mtim + ctim < + data + > + inode_no + next_block
// in bits = 8 + (256*8) + 32 + 32 + 32 + 32 + 8 + 64 + (16*8) + (16*8) + (16*8) + 64 + 64 = 2768 = 346 bytes
#define NODE_SIZE (346)

#define SUPERBLOCKS 1   // number of blocks designated to be part of superblock


typedef struct fs_tree_node {
    uint8_t type;                       //type of node
    char name[256];                         //name of node
    char *fullname;                     //full path of node
    
    uint32_t uid, gid;              // user ID and group IP
    uint32_t perms;                 // file permissions (supposed to be similar to Ubuntu)
    uint8_t nlinks;             // number of links to this
    
    struct fs_tree_node *parent;        //link to parent
    struct fs_tree_node **children;      //links to children
    uint32_t len;                       //number of children
    uint64_t *ch_inodes;            // inode_no of children

    uint8_t *data;						//data for read and write
    uint64_t data_size;						//size of data
    uint64_t block_count;               // number of blocks
    uint64_t inode_no;                  // the inode number, i.e, the block containing first part of data

    struct timespec st_atim;            /* time of last access */
    struct timespec st_mtim;            /* time of last modification */
    struct timespec st_ctim;            /* time of last status change */
}fs_tree_node;

/*
types:
    1 = file
    2 = directory
*/

extern int diskfd;

/*
Free all dynamically allocated members of node and free node itself too.
Return 0 if all okay, else return -1.
*/
int destroy_node(fs_tree_node *node);

/*
Ourput all fields of node using `error_log`. Output will not be displayed if FFS was compiled or run without `d` prefix as in `make dcompile` or `make drun`.
*/
void output_node(fs_tree_node node);

/*
Initialize the tree structure that stores the file system's tree. Creates the root node.
*/
int init_fs();

/*
Uses Depth-First-Search to recursively apply the function (foo) to each node under (curr) and to (curr) itself.
*/
int dfs_dispatch(fs_tree_node *curr, int (*foo)(fs_tree_node *));

/*
Uses Breadth-First-Search to recursively apply the function (foo) to each node under (curr).
*/
int bfs_dispatch(fs_tree_node *curr, int (*foo)(fs_tree_node *));

/*
Returns address of node if node exists in FS tree, else 0.
*/
fs_tree_node *node_exists(const char *path);

/*
Create a file at `path` of type specified by `mode`. If any intermediate directory in `path` doesn't exist, error is thrown automatically.
Returns address of added node in FS tree.
*/
fs_tree_node *add_fs_tree_node(const char *path, uint8_t type);

/*
Remove node at `path`.
If any intermediate directory in `path` doesn't exist, error is thrown.
*/
int remove_fs_tree_node(const char *path);

/*
Copies all members from `from` to `to`, except link to parent, name and fullname.
Assumes both nodes already exist and are allocated space, but pointer members of `to` are not allocated.
Does not free anything, strictly copies and returns.
Returns 0.
*/
int copy_nodes(fs_tree_node *from, fs_tree_node *to);

/*
Load an already initialised FS from a file/persistent storage opened using `openDisk`.
*/
int load_fs(int diskfd);

/*
Rebuilds the FS tree with all the metadata of all files/folders stored in persistent storage accessed via `diskfd`.
*/
void fill_fs_tree(fs_tree_node *root);


#endif