#include "bitmap.h"

// Error logging for THIS MODULE, helps differentiate from logging of other modules
// Prints errors and logging info to STDOUT
// Passes format strings and args to vprintf, basically a wrapper for printf
static void error_log(char *fmt, ...) {
#ifdef ERR_FLAG
    va_list args;
    va_start(args, fmt);
    
    printf("BITMAP : ");
    vprintf(fmt, args);
    printf("\n");

    va_end(args);
#endif
}


int loadBitMap(int fd) {
    error_log("%s called with fd = %d", __func__, fd);

    lseek(fd, 8, SEEK_SET);
    int ret = read(fd, &bmap_size, 8);
    error_log("bmap size = %lu bytes", bmap_size);

    if(ret < 0) {
        error_log("Problem = %d\t read in %s", errno, __func__);
        perror("loadBitMap problem");
        exit(0);
    }

    // bmap_size is number of bytes taken by bitmap
    bitmap = (uint8_t *)malloc(bmap_size);
    if(!bitmap) {
        return -1;
    }

    lseek(fd, SUPERBLOCKS * BLOCK_SIZE, SEEK_SET);
    ret = read(fd, bitmap, bmap_size);

    error_log("Returning with %d", ret);
    return ret;
}


void saveBitMap() {
    error_log("%s called", __func__);

    lseek(diskfd, SUPERBLOCKS * BLOCK_SIZE, SEEK_SET);
    write(diskfd, bitmap, bmap_size);

    error_log("%s done", __func__);
}


uint64_t findFirstFreeBlock() {
    error_log("%s called", __func__);

    uint64_t index = 0;
    int bit_index = 0;
    int val, found = 0;

    while(index < bmap_size) {
        bit_index = 0;
        while(bit_index < 8) {
            val = (int)pow(2, bit_index);
            
            if( !(bitmap[index] & val) ) {
                found = 1;
                break;
            }
            bit_index++;
        }

        if(!found)
            index++;
        else
            break;
    }

    if(found) {
        error_log("Returning with %lu", index * 8 + bit_index);
        return (index * 8 + bit_index);
    }
    
    error_log("Returning not found!");
    return -1;
}


int setBitofMap(uint64_t bitno) {
    error_log("%s called on bitno %llu", __func__, bitno);
    
    uint64_t index = bitno / 8;
    int bit_index = bitno % 8;

    int val = (int)pow(2, bit_index);
    bitmap[index] = bitmap[index] | (val);

    saveBitMap();
    return 0;
}


int clearBitofMap(uint64_t bitno) {
    error_log("%s called on bitno %llu", __func__, bitno);

    uint64_t index = bitno / 8;
    int bit_index = bitno % 8;

    int val = (int)pow(2, bit_index);
    bitmap[index] = bitmap[index] & ~(val);

    saveBitMap();
    return 0;
}

void print_bitmap() {
#ifdef ERR_FLAG
    int index = 0, bit_index = 0;
    for(index = 0; index < bmap_size; index++){
        for (bit_index = 0; bit_index < 8; bit_index++) {
            printf("%d", !!((bitmap[index] << bit_index) & 0x80));
        }
        printf(" ");
    }
    printf("\n");
    ;
#endif
}