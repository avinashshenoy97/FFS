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

extern int diskfd;

#define BLOCK_SIZE 4096                         // block size 4 KB
#define MAX_FILE_SIZE 18446744073709551616    // bytes, largest possible value in unsigned 64 bit int; 13.6 EB (Exabytes)
#define MAX_BLOCK_NO 4503599627370496           //MAX_FILE_SIZE / (4*1024)

/*
Allocate as many blocks as needed to store (n) bytes in data field of node.
Return pointer to begin writing (in-memory).
*/
void *allocate(fs_tree_node *node, uint64_t n);

/*
Like realloc, reallocate data to have blocks needed to store (n) bytes.
Return pointer.
*/
void *reallocate(fs_tree_node *node, uint64_t n);

/*
Like memset for blocks. Set (n) bytes from ptr, to val.
*/
void *setBlocks(void *ptr, uint64_t n, int val);

/*
Deallocate like free.
*/
void deallocate(fs_tree_node *node);


/*
Construct a block to write to disk, metadata (from fs_tree_node) + data
Metadata = type + name + len + uid + gid + perms + nlinks + data_size + atim + mtim + ctim < + data + > + inode_no + next_block

The (block) will contain metadata and all data, last 64 bits of each block left empty to be filled at the time of flushing. The last 64 bits are used to store the block number of the next block where the rest of the data is stored.
Returns the number of blocks allocated and built! The constructed block is placed in `ret`.
*/
uint64_t constructBlock(fs_tree_node *node, void **ret);

/*
Reconstruct node from block at `blockdata` and return the constructed block with all fields filled in correctly. Essentially an inverse of constructBlock.
*/
fs_tree_node *reconstructNode(void *blockdata);

/*
Open a file to be used as a disk and return the file descriptor.
*/
int openDisk(char *filename, int nbytes);

/*
Read one block of data from disk and place the data into `block`. Block number `blocknr` is read from file. Offset is calculated as `blocknr * BLOCK_SIZE`.
*/
int readBlock(uint64_t blocknr, void *block);

/*
Write one block of data to disk from `block`. Block number `blocknr` is written in file. Offset is calculated as `blocknr * BLOCK_SIZE`.
*/
int writeBlock(uint64_t blocknr, void *block);

/*
Essentially a wrapper for writing block data at the first free disk block given by `first` block number, setting the appropriate bit of bitmap, then writing the data to disk. This function also handles cases when a file/folder's data exceed one block, in which case, after writing every block, the next free block is found and written to after filling all the appropriate `next` fields and such.
*/
uint64_t diskWriter(void *blocks_data, uint64_t blocks, uint64_t first);

/*
Essentially a wrapper for reading a block of data from disk and reconstructing the node using appropriate functions. The reconstructed node is returned. This function also handles cases when the block is found to have a `next` field with more data to be read.
NOTE : This function does not read file contents, i.e data, to the FS node.
*/
fs_tree_node *diskReader(uint64_t block);

/*
This function reads the DATA into the FS tree node at `node` handling cases where data may be spread across several blocks.
*/
uint64_t dataDiskReader(fs_tree_node *node);


#endif