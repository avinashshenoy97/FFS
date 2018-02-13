/*
	1. Create the file
	2. Create superblock
	3. Write size of disk as first 64 bits of file
	4. Write size of bitmap (in blocks) required for entire disk in next 64 bits
	5. Create root ("/") directory
	6. Mark blocks used by superblock, bitmap and root directory as 1 in bitmap
	7. Write bitmap to file
	8. Write root node to file
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>

#include "tree.h"
#include "disk.h"
#include "bitmap.h"

// macros for backward compatibility
#define openDisk(x) openDisk(x, 0)

int diskfd;

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

int main(int argc, char **argv){
	init_fs();		// Creates root directory

	uint64_t i, size = 1 * 1024 * 1024;	// 1 MB
	int fd = open(argv[1], O_CREAT | O_TRUNC | O_RDWR, 0666);
	diskfd = fd;

	uint8_t to_write = 0;
	for(i = 0 ; i < size ; i++) {
		write(fd, &to_write, sizeof(to_write));
	}
	printf("%s\n", "Done creating! Writing superblock and metadata!");

	// Write size of disk to superblock
	lseek(fd, 0, SEEK_SET);
	write(fd, &size, sizeof(size));
	error_log("Wrote size %lu to file\n", size);

	// Calculate size of BITMAP in bits
	uint64_t bsize = size / BLOCK_SIZE;
	// Size of BITMAP in bytes
	bsize /= 8;	
	bmap_size = bsize;
	error_log("bsize %lu to file\n", bsize);

	// Write number of blocks taken by bitmap in superblock
	lseek(fd, sizeof(size), SEEK_SET);
	write(fd, &bsize, sizeof(bsize));
	error_log("Wrote bsize %lu to file\n", bsize);

	// Blocks needed by BITMAP, to be marked as 1 in bitmap
	uint64_t bmap_blocks = bsize / BLOCK_SIZE;
	bmap_blocks++;

	// First (bmap_blocks) need to marked with 1 in BITMAP
	error_log("Marking first %lu blocks\n", bmap_blocks + SUPERBLOCKS);

	bitmap = calloc(bsize, BLOCK_SIZE);
	if(!bitmap) {
		perror("No memory for bitmap");
		exit(0);
	}
	for(i = 0 ; i < bmap_blocks + SUPERBLOCKS ; i++)
		setBitofMap(i);

	error_log("Done marking!\n");

	void *buf;
	uint64_t firstFreeBlock = findFirstFreeBlock();
	error_log("First free block = %lu\n", firstFreeBlock);
	error_log("Constructing block for root node!\n");
	
	fs_tree_node *root = node_exists("/");
	root->inode_no = firstFreeBlock;
	
	constructBlock(root, &buf);		// Create block for root node
	error_log("Done constructing block for root node!\n");
	output_node(*root);

	writeBlock(firstFreeBlock, buf);
	error_log("Done writing block for root node!\n");
	
	setBitofMap(firstFreeBlock);
	error_log("Writing bitmap to file\n");
	for(i = 0 ; i < bmap_blocks ; i++) {
		writeBlock(SUPERBLOCKS + i, bitmap + (i * BLOCK_SIZE));
	}
	
	error_log("Freeing, closing, end!\n");
	free(buf);
	free(bitmap);
	close(fd);
	printf("Done!\n");
	return 0;
}
