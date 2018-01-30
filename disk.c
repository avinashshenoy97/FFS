#include "disk.h"


// Error logging for THIS MODULE, helps differentiate from logging of other modules
// Prints errors and logging info to STDOUT
// Passes format strings and args to vprintf, basically a wrapper for printf
static void error_log(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    printf("DISK : ");
    vprintf(fmt, args);
    printf("\n");

    va_end(args);
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