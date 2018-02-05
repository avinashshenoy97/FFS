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


#define DEF_DIR_PERM (0775)
#define DEF_FILE_PERM (0664)

// fields = type + name + uid + gid + perms + nlinks + data_size + atim + mtim + ctim + next_block
// in bits = 8 + (256*8) + 32 + 32 + 32 + 8 + 64 + (16*8) + (16*8) + (16*8) + 64 = 2672 = 334 bytes
#define NODE_SIZE (334)

#define SUPERBLOCKS 1   // number of blocks designated to be part of superblock


typedef struct fs_tree_node {
    uint8_t type;                       //type of node
    char *name;                         //name of node
    char *fullname;                     //full path of node
    
    uint32_t uid, gid;              // user ID and group IP
    uint32_t perms;                 // file permissions (supposed to be similar to Ubuntu)
    uint8_t nlinks;             // number of links to this
    
    struct fs_tree_node *parent;        //link to parent
    struct fs_tree_node **children;      //links to children
    uint32_t len;                       //number of children

    uint8_t *data;						//data for read and write
    uint64_t data_size;						//size of data
    uint64_t block_count;               // number of blocks

    struct timespec st_atim;            /* time of last access */
    struct timespec st_mtim;            /* time of last modification */
    struct timespec st_ctim;            /* time of last status change */
}fs_tree_node;

/*
types:
    1 = file
    2 = directory
*/

int destroy_node(fs_tree_node *node);        //free all parts a node from the FS tree
int init_fs();              //initialize FS tree with root
int dfs_dispatch(fs_tree_node *curr, int (*foo)(fs_tree_node *));   // apply a function to a node and its children recursively


fs_tree_node *node_exists(const char *path);        //check if node exists
fs_tree_node *add_fs_tree_node(const char *path, uint8_t type);     //add a node to FS tree at path
int remove_fs_tree_node(const char *path);          //remove a node from FS tree
int copy_nodes(fs_tree_node *from, fs_tree_node *to);       //copy all members from (from) to (to)

#endif