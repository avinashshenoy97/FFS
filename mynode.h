#ifndef MYNODE_H

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


#define MAX_FILE_SIZE 4294967295    //largest possible value in unsigned 32 bit int


struct mynode {
    u_int8_t links;             //links count
    uint16_t mode;              //file mode
    uint16_t uid, gid;          //user id and group id
    uint32_t size;              //size in bytes
    uint32_t flags;             //file flags
    uint32_t blocks;            //blocks count
    time_t atime, ctime;        //access time, change time
    time_t mtime, dtime;        //modification time, deletion time
    
};

#endif