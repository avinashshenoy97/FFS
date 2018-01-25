#ifndef FFS_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>


typedef struct fs_tree_node {
    uint8_t type;                       //type of node
    char *name;                         //name of node
    char *fullname;                     //full path of node
    struct fs_tree_node *children;      //links to children
    uint32_t len;                       //number of children
    struct fs_tree_node *parent;        //link to parent
}fs_tree_node;

/*
types:
    1 = file
    2 = firectory
*/

int init_fs();
fs_tree_node *node_exists(const char *path);
int add_fs_tree_node(const char *path, short type);

#endif