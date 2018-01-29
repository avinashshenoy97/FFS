#ifndef FFS_OPERATIONS_H
#define FFS_OPERATIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <fuse.h>


int ffs_getattr(const char *path, struct stat *s);
int ffs_mknod(const char *path, mode_t m, dev_t d);
int ffs_mkdir(const char *path, mode_t m);
int ffs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int ffs_rmdir(const char *path);
int ffs_open(const char *path, struct fuse_file_info *fi);
int ffs_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi);
int ffs_write(const char *path, const char *buf, size_t size, off_t offset,struct fuse_file_info *fi);
int ffs_utimens(const char *path, const struct timespec tv[2]);
int ffs_truncate(const char* path, off_t size);

#endif
