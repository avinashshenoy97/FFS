#include "disk.h"
#include<unistd.h>

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


void *setBlocks(void *ptr, uint64_t n, int val) {
    error_log("%s called on %p for %ld bytes to set %d", __func__, ptr, n, val);

    n = n * BLOCK_SIZE;
    
    return memset(ptr, n, val);
}


void deallocate(fs_tree_node *node) {
    error_log("%s called on %p", __func__);

    if(!node->data)
        free(node->data);
    
    node->data_size = 0;
    node->block_count = 0;
    node->data = NULL;
}


uint64_t constructBlock(fs_tree_node *node, void **ret) {
    error_log("%s called on %p", __func__, node);
    
    uint64_t space_needed;
    switch(node->type) {
        case 1:
            space_needed = NODE_SIZE + node->data_size;
            break;

        case 2:
            space_needed = NODE_SIZE + (node->len * sizeof(node->inode_no));
            break;
    }
    uint64_t blocks_needed = (space_needed / BLOCK_SIZE) + 1;
    // in every block, last 64 bytes are for storing next block number
    // so additional space required =
    space_needed += (blocks_needed * 64);

    // NODE_SIZE accounts for the next block field in the first block
    space_needed -= 64;

    blocks_needed = (space_needed / BLOCK_SIZE) + 1;    // Re-calculate blocks needed after adding

    error_log("Space need = %lu\tBlocks needed = %lu", space_needed, blocks_needed);

    void *store = malloc(blocks_needed * BLOCK_SIZE);
    if(!store) {
        error_log("NO MEMORY!");
        return -ENOMEM;
    }
    error_log("Storage allocated %p", store);

    uint64_t alloc = 0;     //bytes in (store) already allocated
    uint64_t data_copied = 0;   //bytes of data copied
    
    memcpy(store + alloc, &(node->type), sizeof(node->type));
    alloc += sizeof(node->type);

    error_log("Done writing type, alloc = %d", alloc);

    memcpy(store + alloc, &(node->name), sizeof(node->name));
    alloc += sizeof(node->name);

    error_log("Done writing name %s, alloc = %d", node->name, alloc);

    memcpy(store + alloc, &(node->len), sizeof(node->len));
    alloc += sizeof(node->len);

    error_log("Done writing len %u, alloc = %d", node->len, alloc);

    memcpy(store + alloc, &(node->uid), sizeof(node->uid));
    alloc += sizeof(node->uid);

    error_log("Done writing uid, alloc = %d", alloc);

    memcpy(store + alloc, &(node->gid), sizeof(node->gid));
    alloc += sizeof(node->gid);

    error_log("Done writing gid, alloc = %d", alloc);

    memcpy(store + alloc, &(node->perms), sizeof(node->perms));
    alloc += sizeof(node->perms);

    error_log("Done writing perms, alloc = %d", alloc);

    memcpy(store + alloc, &(node->nlinks), sizeof(node->nlinks));
    alloc += sizeof(node->nlinks);

    error_log("Done writing nlinks, alloc = %d", alloc);

    memcpy(store + alloc, &(node->data_size), sizeof(node->data_size));
    alloc += sizeof(node->data_size);

    error_log("Done writing data_size %lu, alloc = %d", node->data_size, alloc);

    memcpy(store + alloc, &(node->st_atim), sizeof(node->st_atim));
    alloc += sizeof(node->st_atim);

    error_log("Done writing atime, alloc = %d", alloc);

    memcpy(store + alloc, &(node->st_mtim), sizeof(node->st_mtim));
    alloc += sizeof(node->st_mtim);

    error_log("Done writing mtime, alloc = %d", alloc);

    memcpy(store + alloc, &(node->st_ctim), sizeof(node->st_ctim));
    alloc += sizeof(node->st_ctim);

    error_log("Done writing ctime, alloc = %d", alloc);

    memcpy(store + alloc, &(node->inode_no), sizeof(node->inode_no));
    alloc += sizeof(node->inode_no);

    error_log("Done writing inode no %d, alloc = %d", node->inode_no, alloc);

    // nothing to copy for next block, have to set manually later
    uint64_t blocks_done = 0;
    switch(node->type) {
        case 1:
            // copy data
            if(node->data_size <= (BLOCK_SIZE - NODE_SIZE)) {
                error_log("Writing at alloc = %lu, data_size %lu \t %4s", alloc, node->data_size, node->data);
                memcpy(store + alloc, (node->data), (node->data_size));
                alloc += (node->data_size);
                data_copied += (node->data_size);
            }
            else {
                error_log("Writing at alloc = %lu, data_size %lu", alloc, BLOCK_SIZE - NODE_SIZE);
                memcpy(store + alloc, (node->data), (BLOCK_SIZE - NODE_SIZE));
                alloc += (BLOCK_SIZE - NODE_SIZE);
                data_copied += (BLOCK_SIZE - NODE_SIZE);
                alloc += 8;
            }

            error_log("Done writing inline data, alloc = %d", alloc);
            while(data_copied < node->data_size && (node->data_size - data_copied) > (BLOCK_SIZE - 8)) {
                error_log("Writing at alloc = %lu, data_size %lu", alloc, BLOCK_SIZE - 8);
                memcpy(store + alloc, (node->data) + data_copied, BLOCK_SIZE - 8);
                alloc += (BLOCK_SIZE);
                data_copied += (BLOCK_SIZE - 8);
            }

            if((node->data_size - data_copied) > 0) {
                error_log("Writing at alloc = %lu, data_size %lu", alloc, (node->data_size - data_copied));
                memcpy(store + alloc, (node->data) + data_copied, (node->data_size - data_copied));
                alloc += ((node->data_size - data_copied));
                data_copied += (node->data_size - data_copied);
            }
        break;

        case 2:
            //copy child inode nos
            error_log("case 2 %d %lu", node->type, node->len);
            for(data_copied = 0 ; data_copied < node->len ; data_copied++) {
                memcpy(store + alloc, &(node->ch_inodes[data_copied]), sizeof(node->inode_no));
                alloc += sizeof(node->inode_no);
                error_log("Done writing child inode %lu, alloc = %lu", node->ch_inodes[data_copied], alloc);

                if((alloc + 8) > (BLOCK_SIZE * blocks_done - 8)) {
                    blocks_done++;
                    alloc = blocks_done * BLOCK_SIZE;
                }
            }
        break;
    }

    error_log("Done writing data, alloc = %d", alloc);

    *ret = store;

    error_log("Returning with %d", blocks_needed);
    return blocks_needed;
}


fs_tree_node *reconstructNode(void *blockdata) {
    error_log("%s called", __func__);

    fs_tree_node *node = (fs_tree_node *)malloc(sizeof(fs_tree_node));
    if(!node) {
        error_log("no memory for node");
        return (fs_tree_node *)(-ENOMEM);
    }

    uint64_t alloc = 0;     //bytes copied from blockdata
    
    memcpy(&(node->type), blockdata + alloc, sizeof(node->type));
    alloc += sizeof(node->type);

    error_log("Done reading type, alloc = %d", alloc);

    memcpy(&(node->name), blockdata + alloc, sizeof(node->name));
    alloc += sizeof(node->name);

    error_log("Done reading name %s, alloc = %d", node->name, alloc);

    memcpy(&(node->len), blockdata + alloc, sizeof(node->len));
    alloc += sizeof(node->len);

    error_log("Done reading len %u, alloc = %d", node->len, alloc);

    memcpy(&(node->uid), blockdata + alloc, sizeof(node->uid));
    alloc += sizeof(node->uid);

    error_log("Done reading uid, alloc = %d", alloc);
    error_log("Len is still %u", node->len);

    memcpy(&(node->gid), blockdata + alloc, sizeof(node->gid));
    alloc += sizeof(node->gid);

    error_log("Done reading gid, alloc = %d", alloc);
    error_log("Len is still %u", node->len);

    memcpy(&(node->perms), blockdata + alloc, sizeof(node->perms));
    alloc += sizeof(node->perms);

    error_log("Done reading perms, alloc = %d", alloc);
    error_log("Len is still %u", node->len);

    memcpy(&(node->nlinks), blockdata + alloc, sizeof(node->nlinks));
    alloc += sizeof(node->nlinks);

    error_log("Done reading nlinks, alloc = %d", alloc);
    error_log("Len is still %u", node->len);

    memcpy(&(node->data_size), blockdata + alloc, sizeof(node->data_size));
    alloc += sizeof(node->data_size);

    error_log("Done reading data_size %lu, alloc = %d", node->data_size, alloc);

    memcpy(&(node->st_atim), blockdata + alloc, sizeof(node->st_atim));
    alloc += sizeof(node->st_atim);

    error_log("Done reading atime, alloc = %d", alloc);

    memcpy(&(node->st_mtim), blockdata + alloc, sizeof(node->st_mtim));
    alloc += sizeof(node->st_mtim);

    error_log("Done reading mtime, alloc = %d", alloc);

    memcpy(&(node->st_ctim), blockdata + alloc, sizeof(node->st_ctim));
    alloc += sizeof(node->st_ctim);

    error_log("Done reading ctime, alloc = %d", alloc);

    memcpy(&(node->inode_no), blockdata + alloc, sizeof(node->inode_no));
    alloc += sizeof(node->inode_no);
    error_log("Done reading inode %lu, alloc = %d", node->inode_no, alloc);

    node->ch_inodes = (uint64_t *)malloc(sizeof(uint64_t) * node->len);
    error_log("Starting to load child inodes");

    uint64_t i, temp, blocks_done = 0;
    for(i = 0 ; i < node->len ; i++) {
        memcpy(&temp, blockdata + alloc, sizeof(node->inode_no));
        alloc += sizeof(node->inode_no);
        node->ch_inodes[i] = temp;
        error_log("Child %lu = %lu", i, temp);

        if((alloc + 8) > (BLOCK_SIZE - 8))
            alloc = BLOCK_SIZE * ++blocks_done;
    }

    return node;
}


int openDisk(char *filename, int nbytes) {
    error_log("%s called on %s", __func__, filename);
    
    int fd = open(filename, O_RDWR, 0666);
    if(fd < 0) {
        error_log("Problem = %d\t in %s", errno, __func__);
        perror("openDisk problem");
        exit(0);
    }

    error_log("Returning with fd = %d", fd);
    return fd;
}


int readBlock(uint64_t blocknr, void *block) {
    error_log("%s called on fd : %d for block %d", __func__, diskfd, blocknr);

    int ret;
    if(blocknr < MAX_BLOCK_NO){
        error_log("Reading %d from offset %d", BLOCK_SIZE, blocknr * BLOCK_SIZE);
        lseek(diskfd, blocknr * BLOCK_SIZE, SEEK_SET);
        ret = read(diskfd, block, BLOCK_SIZE);
    }
    else{
        return -EPERM;
    }

    error_log("Returning with %d", ret);
    return ret;
}


int writeBlock(uint64_t blocknr, void *block) {
    error_log("%s called on fd : %d for block %d", __func__, diskfd, blocknr);
    
    int ret;
    if(blocknr < MAX_BLOCK_NO){
        error_log("Writing at off = %llu; size = %llu", (blocknr) * BLOCK_SIZE, BLOCK_SIZE);
        lseek(diskfd, (blocknr) * BLOCK_SIZE, SEEK_SET);
        ret = write(diskfd, block, BLOCK_SIZE);
    }
    else{
        return -EPERM;
    }

    error_log("Returning with %d", ret);
    return ret;
}


uint64_t diskWriter(void *blocks_data, uint64_t blocks, uint64_t first) {
    error_log("%s called on fd : %d for blocks %lu from first %lu", __func__, diskfd, blocks, first);

    uint64_t i, toWrite, next = first;
    for(i = 0 ; i < blocks ; i++) {
        toWrite = next;
        setBitofMap(toWrite);
        next = 0;
        if(i != (blocks - 1))
            next = findFirstFreeBlock();
        
        memcpy(blocks_data + (BLOCK_SIZE * (i+1) - 8), &next, sizeof(next)); // set the next block field
        writeBlock(toWrite, blocks_data + (i * BLOCK_SIZE));
    }

    error_log("Freeing input buffer after write %p", blocks_data);
    //free(blocks_data); // throws error "invalid pointer" = reason unkown
    error_log("Returning with %d", i);

    return i;
}


fs_tree_node *diskReader(uint64_t block) {
    error_log("%s called on fd : %d from block %d", __func__, diskfd, block);
    
    void *buf = calloc(sizeof(uint8_t), BLOCK_SIZE);
    readBlock(block, buf);
    fs_tree_node *node = reconstructNode(buf);
    
    node->fullname = NULL;
    node->parent = NULL;
    node->children = NULL;
    node->data = NULL;

    if(!buf)
        free(buf);
    error_log("Returning with node = %p and len = %u", node, node->len);
    return node;
}


uint64_t dataDiskReader(fs_tree_node *node) {
    error_log("%s called on node : %p ", __func__, node);

    uint64_t data_read = 0, next = 0;
    node->data = (uint8_t *)malloc(sizeof(uint8_t) * node->data_size);
    void *buf = malloc(sizeof(uint8_t *) * BLOCK_SIZE);
    readBlock(node->inode_no, buf);

    if(node->data_size <= (BLOCK_SIZE - NODE_SIZE)) {
        error_log("1Copying %u", node->data_size);
        memcpy(node->data, buf + NODE_SIZE - 8, node->data_size);
        data_read += node->data_size;
    }
    else {
        error_log("2Copying %u", BLOCK_SIZE - NODE_SIZE);
        memcpy(node->data, buf + NODE_SIZE - 8, BLOCK_SIZE - NODE_SIZE);
        data_read += BLOCK_SIZE - NODE_SIZE;
        memcpy(&next, buf + BLOCK_SIZE - 8, sizeof(next));
    }
    
    error_log("Done reading inline data = %d \tnext = %d", data_read, next);

    while(next) {
        readBlock(next, buf);
        if((node->data_size - data_read) >= (BLOCK_SIZE - 8)) {
            error_log("3Copying %u", BLOCK_SIZE - 8);
            memcpy(node->data + data_read, buf, BLOCK_SIZE - 8);
            memcpy(&next, buf + BLOCK_SIZE - 8, sizeof(next));
        }
        else {
            error_log("4Copying %u", node->data_size - data_read);
            memcpy(node->data + data_read, buf, (node->data_size - data_read));
            memcpy(&next, buf + BLOCK_SIZE - 8, sizeof(next));
        }

        error_log("Done reading more data = %d \tnext = %d", data_read, next);
    }

    error_log("Done with %lu", data_read);
    return data_read;
}
