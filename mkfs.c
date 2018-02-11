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

int main(int argc, char **argv){
	init_fs();		// Creates root directory

	uint64_t i, size = 1 * 1024 * 1024;	// 1 MB
	int fd = open(argv[1], O_CREAT | O_TRUNC | O_RDWR, 0666);
	diskfd = fd;

	uint8_t to_write = 0;
	for(i = 0 ; i < size ; i++) {
		write(fd, &to_write, sizeof(to_write));
	}
	printf("%s\n", "Done creating!");

	// Write size of disk to superblock
	lseek(fd, 0, SEEK_SET);
	write(fd, &size, sizeof(size));
	printf("Wrote size %llu to file\n", size);

	// Calculate size of BITMAP in bits
	uint64_t bsize = size / BLOCK_SIZE;
	// Size of BITMAP in bytes
	bsize /= 8;
	// Size of BITMAP in blocks
	bsize /= BLOCK_SIZE;
	bsize++;
	printf("bsize %llu to file\n", bsize);
	bmap_size = bsize;

	// Write number of blocks taken by bitmap in superblock
	lseek(fd, sizeof(size), SEEK_SET);
	write(fd, &bsize, sizeof(bsize));
	printf("Wrote bsize %llu to file\n", bsize);

	// Blocks needed by BITMAP, to be marked as 1 in bitmap
	uint64_t bmap_blocks = bsize / BLOCK_SIZE;
	bmap_blocks++;

	// First (bmap_blocks) need to marked with 1 in BITMAP
	printf("Marking first %d blocks\n", bmap_blocks + SUPERBLOCKS);

	bitmap = calloc(bsize, BLOCK_SIZE);
	if(!bitmap) {
		perror("No memory for bitmap");
		exit(0);
	}
	for(i = 0 ; i < bmap_blocks + SUPERBLOCKS ; i++)
		setBitofMap(i);

	printf("Done marking!\n");

	void *buf;
	uint64_t firstFreeBlock = findFirstFreeBlock();
	printf("First free block = %d\n", firstFreeBlock);
	printf("Constructing block for root node!\n");
	
	fs_tree_node *root = node_exists("/");
	root->inode_no = firstFreeBlock;
	constructBlock(root, &buf);		// Create block for root node

	output_node(*root);
	
	printf("FIRST free block = %d\n", firstFreeBlock);
	//memcpy(buf + (BLOCK_SIZE - 8*2), &firstFreeBlock, sizeof(firstFreeBlock)); // set the inode field
	printf("FIRST free block = %d\n", firstFreeBlock);

	printf("Done constructing block for root node!\n");
	writeBlock(firstFreeBlock, buf);
	printf("FIRST free block = %d\n", firstFreeBlock);

	printf("Done writing block for root node!\n");
	setBitofMap(firstFreeBlock);
	printf("Writing bitmap to file\n");
	for(i = 0 ; i < bmap_blocks ; i++) {
		writeBlock(SUPERBLOCKS + i, bitmap + (i * BLOCK_SIZE));
	}
	
	printf("Freeing, closing, end!\n");
	free(buf);
	close(fd);
	return 0;
}
