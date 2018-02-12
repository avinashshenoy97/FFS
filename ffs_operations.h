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

#include "bitmap.h"
#include "tree.h"
#include "disk.h"

/*
Get attributes function. Used to get attributes of a file/folder, i.e, FS tree node and "convert" them to the stat structure understood by Linux.
This function retrieves the FS tree node at "path" and fills the attribtues in the appropriate fields in the `struct stat` passed. Commonly used by running `stat` or `ls` on bash shell. This function is used by FUSE and OS to verify the existence of a file/directory.
Returns 0 if successful, else returns the appropriate error as defined in `errno.h`.
*/
int ffs_getattr(const char *path, struct stat *s);

/*
MKNOD function. Used to create a file that doesn't exist. Newer Linux Kernel versions will attempt to call CREATE; when that fails, MKNOD is used. Serves essentially the same purpose.
A file is created at `path` with mode `m`. Device is assumed to be current device, parameter unused. Commonly used by running `touch` on bash shell.
Returns 0 if successful, else returns the appropriate error as defined in `errno.h`.
*/
int ffs_mknod(const char *path, mode_t m, dev_t d);

/*
MKDIR function. Used to create directories that do not exist.
Directory is created at `path` with mode `m`. Commonly used by running `mkdir` on bash shell.
Returns 0 if successful, else returns the appropriate error as defined in `errno.h`.
*/
int ffs_mkdir(const char *path, mode_t m);

/*
READDIR function. Used to read contents of a directory.
This function reads contents of the directory at `path` and places the same into the buffer `buf` using the filler function `filler` from offset `offset`. The `filler` function and `buffer` are taken care of by FUSE. Commonly used by running `ls` on bash shell.
Returns 0 if successful, else returns the appropriate error as defined in `errno.h`.
*/
int ffs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);

/*
RMDIR function. Used to remove a directory.
This function finds the FS tree node at `path` and removes it, if it is empty. If not, an error is returned. This is taken care of by FUSE or OS. Commonly used by running `rmdir` on bash shell.
Returns 0 if successful, else returns the appropriate error as defined in `errno.h`.
*/
int ffs_rmdir(const char *path);

/*
RENAME function. Used to move or rename a file/directory.
This function moves the FS tree node from `from` to `to`. Commonly used by running `mv` on bash shell.
Returns 0 if successful, else returns the appropriate error as defined in `errno.h`.
*/
int ffs_rename(const char *from, const char *to);

/*
OPEN function. Used to open a file. This function is used before reading/writing to a file via any program.
Returns 0 if successful, else returns the appropriate error as defined in `errno.h`.
*/
int ffs_open(const char *path, struct fuse_file_info *fi);

/*
READ function. Used to read contents of a file.
This function finds the FS tree node at `path` and places it's contents (data) in `buf`. Commonly used by programs when `read` system call is used.
Returns 0 if successful, else returns the appropriate error as defined in `errno.h`.
*/
int ffs_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi);

/*
WRITE function. Used to write contents to a file.
This function finds the FS tree node at `path` and places the contents of `buf` into it. Commonly used by programs when `write` system call is used.
Returns 0 if successful, else returns the appropriate error as defined in `errno.h`.
*/
int ffs_write(const char *path, const char *buf, size_t size, off_t offset,struct fuse_file_info *fi);

/*
UTIMENS function. Used to change access, modified, etc times displayed in Linux stat structure.
This function finds the FS tree node at `path` and copies the access and modified time from `tv` into the node. Commonly used by programs when `touch` command is run on bash shell.
Returns 0 if successful, else returns the appropriate error as defined in `errno.h`.
*/
int ffs_utimens(const char *path, struct utimbuf *tv);

/*
TRUNCATE function. Used to truncate contents of a file, or resize the data of a file.
This function finds the FS tree node at `path` and changed the size of it's contents (data) to `size`. Commonly used by programs when `O_TRUNC` is passed to `open` system call to remove all data from file.
Returns 0 if successful, else returns the appropriate error as defined in `errno.h`.
*/
int ffs_truncate(const char* path, off_t size);

/*
UNLINK function. Used to delete a file.
This function finds the FS tree node at `path` and removes it from the tree. Commonly used by running `rm` on bash shell.
Returns 0 if successful, else returns the appropriate error as defined in `errno.h`.
*/
int ffs_unlink(const char *path);

/*
CHMOD function. Used to change permissions of a file.
This function finds the FS tree node at `path` and changes it's permissions to `setPerm`. Commonly used by running `chmod` on bash shell.
Returns 0 if successful, else returns the appropriate error as defined in `errno.h`.
*/
int ffs_chmod(const char *path, mode_t setPerm);

/*
CHOWN function. Used to change ownership of a file.
This function finds the FS tree node at `path` and changes it's owner user id or group id to `u` or `g` respectively. Commonly used by running `chown` on base shell.
Returns 0 if successful, else returns the appropriate error as defined in `errno.h`.
*/
int ffs_chown(const char *path, uid_t u, gid_t g);

/*
FLUSH function. Used to flush contents of a file to disk, i.e, persistent storage.
This function writes the contents of a file to disk. Commonly used when programs `close` a file.
Returns 0 if successful, else returns the appropriate error as defined in `errno.h`.
*/
int ffs_flush(const  char *path, struct fuse_file_info *fi);

#endif
