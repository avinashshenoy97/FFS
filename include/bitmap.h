#ifndef BITMAP_H
#define BITMAP_H
/*
    Responsible for managing and manipulating the bitmap.
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "disk.h"

#define MAP_BLOCK (8)

extern int diskfd;

uint64_t bmap_size;     // Size of BITMAP in bytes
uint8_t *bitmap;

/*
Load bitmap from file via `fd` and place the pointer in global `bitmap`. Also fill `bmap_size` to indicate size of bitmap. Bitmap is always stored just after the superblock in the disk.
*/
int loadBitMap(int fd);

/*
Save bitmap to file via `fd` from the pointer global `bitmap`. Bitmap is always stored just after the superblock in the disk.
*/
void saveBitMap();

/*
Find first free block in the disk, i.e, the first 0 bit in the bitmap.
*/
uint64_t findFirstFreeBlock();

/*
Set the bit `bitno` of `bitmap` to 1.
*/
int setBitofMap(uint64_t bitno);

/*
Clear the bit `bitno` of `bitmap`, i.e, set it to 0.
*/
int clearBitofMap(uint64_t bitno);

/*
Print the bitmap. Each bit's value is printed. Output will not be shown unless FFS was compiled/run with `d` prefix as in `make dcompile` or `make drun`.
*/
void print_bitmap();

#endif