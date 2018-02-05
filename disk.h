#ifndef DISK_H
#define DISK_H
/*
    To be used only for data, not metadata!
    Responsible for setting block_count in nodes
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <fuse.h>
#include "tree.h"

typedef struct fs_tree_node fs_tree_node;

#define BLOCK_SIZE 4096                         // block size 4 KB
#define MAX_FILE_SIZE 18446744073709551616    // bytes, largest possible value in unsigned 64 bit int; 13.6 EB (Exabytes)
#define MAX_BLOCK_NO 4503599627370496           //MAX_FILE_SIZE / (4*1024)

void *allocate(fs_tree_node *node, uint64_t n);         // allocate blocks required for (n) bytes
void *reallocate(fs_tree_node *node, uint64_t n);         // reallocate orig to have blocks required for (n) bytes
void *setBlocks(void *ptr, uint64_t n, int val);              // set (n) bytes from ptr, to val
void deallocate(fs_tree_node *node);             // do what free does

uint64_t constructBlock(fs_tree_node *noden void **ret)               //construct blocks of data to be used by writeBlock

int openDisk(char *filename, int nbytes);
int readBlock(int disk, int blocknr, void *block);
int writeBlock(int disk, int blocknr, void *block);


#endif