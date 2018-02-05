#include "disk.h"


// Error logging for THIS MODULE, helps differentiate from logging of other modules
// Prints errors and logging info to STDOUT
// Passes format strings and args to vprintf, basically a wrapper for printf
static void error_log(char *fmt, ...) {
#ifdef ERR_FLAG
    va_list args;
    va_start(args, fmt);
    
    printf("DISK : ");
    vprintf(fmt, args);
    printf("\n");

    va_end(args);
#endif
}

/*
Allocate as many blocks as needed to store (n) bytes in data field of node
Return pointer or block number to begin writing (depends on in-memory or in-disk)
*/
void *allocate(fs_tree_node *node, uint64_t n) {
    error_log("%s called on %p for %ld bytes", __func__, node, n);

    n = n / (4 * 1024);
    n += 1;
    error_log("Going to allocate %ld blocks", n);
    // n blocks are to be allocated

    node->data = (uint8_t *)malloc(n * BLOCK_SIZE);

    if(!node->data)
        return (void *)(-ENOMEM);

    node->block_count = n;
    return (void *)(node->data);
}

/*
Like realloc, reallocate data to have blocks needed to store (n) bytes
Return pointer or block number
*/
void *reallocate(fs_tree_node *node, uint64_t n) {
    error_log("%s called on %p for %ld bytes", __func__, node, n);

    n = n / (4 * 1024);
    n += 1;
    // n blocks are to be allocated
    error_log("Going to allocate %ld blocks", n);
    void *temp_buf;

    if(!node->data)
        temp_buf = malloc(n * BLOCK_SIZE);
    else
        temp_buf = realloc(node->data, n * BLOCK_SIZE);

    if(!temp_buf) {
        return NULL;
    }
    else
        node->data = temp_buf;

    node->block_count = n;
    return node->data;
}

/*
Like memset for blocks
*/
void *setBlocks(void *ptr, uint64_t n, int val) {
    error_log("%s called on %p for %ld bytes to set %d", __func__, ptr, n, val);

    n = n * BLOCK_SIZE;
    
    return memset(ptr, n, val);
}

/*
Deallocate like free
*/
void deallocate(fs_tree_node *node) {
    error_log("%s called on %p", __func__);

    free(node->data);
    
    node->data_size = 0;
    node->block_count = 0;
    node->data = NULL;
}

/*
Construct a block to write to disk, metadata (from fs_tree_node) + data
Metadata = type + name + uid + gid + perms + nlinks + data_size + atim + mtim + ctim + next_block

The (block) will contain metadata and all data, last 64 bytes of each block left empty to be filled at the time of flushing
Returns the number of blocks allocated and built!
*/
uint64_t constructBlock(fs_tree_node *noden void **ret) {
    error_log("%s called on %p", __func__);
    
    uint64_t space_needed = NODE_SIZE + data_size;
    uint64_t blocks_needed = (space_needed / BLOCK_SIZE) + 1;
    // in every block, last 64 bytes are for storing next block number
    // so additional space required =
    space_needed += (blocks_needed * 64);

    // NODE_SIZE accounts for the next block field in the first block
    space_needed -= 64;

    void *store = malloc(space_needed);
    if(!store) {
        error_log("NO MEMORY!");
        return -ENOMEM;
    }
    error_log("Storage allocated");

    uint64_t alloc = 0;     //bytes in (store) already allocated
    uint64_t data_copied = 0;   //bytes of data copied
    
    memcpy(store + alloc, &(node->type), sizeof(node->type));
    alloc += sizeof(node->type);

    memcpy(store + alloc, &(node->name), sizeof(node->name));
    alloc += sizeof(node->name);

    memcpy(store + alloc, &(node->uid), sizeof(node->uid));
    alloc += sizeof(node->uid);

    memcpy(store + alloc, &(node->gid), sizeof(node->gid));
    alloc += sizeof(node->gid);

    memcpy(store + alloc, &(node->perms), sizeof(node->perms));
    alloc += sizeof(node->perms);

    memcpy(store + alloc, &(node->nlinks), sizeof(node->nlinks));
    alloc += sizeof(node->nlinks);

    memcpy(store + alloc, &(node->data_size), sizeof(node->data_size));
    alloc += sizeof(node->data_size);

    memcpy(store + alloc, &(node->atim), sizeof(node->atim));
    alloc += sizeof(node->atim);

    memcpy(store + alloc, &(node->mtim), sizeof(node->mtim));
    alloc += sizeof(node->mtim);

    memcpy(store + alloc, &(node->ctim), sizeof(node->ctim));
    alloc += sizeof(node->ctim);

    // nothing to copy for next block, have to set manually later

    // copy data
    memcpy(store + alloc, &(node->data), (BLOCK_SIZE - NODE_SIZE));
    alloc += (BLOCK_SIZE - NODE_SIZE);
    data_copied += (BLOCK_SIZE - NODE_SIZE);

    while(data_copied < node->data_size) {
        memcpy(store + alloc, &(node->data) + data_copied, BLOCK_SIZE - 64);
        alloc += (BLOCK_SIZE - 64);
        data_copied += (BLOCK_SIZE - 64);
    }

    *ret = store;

    return blocks_needed;
}