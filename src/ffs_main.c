#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <fuse.h>
#include "ffs_operations.h"
#include "tree.h"


// macros for backward compatibility
#define openDisk(x) openDisk(x, 0)

char *path_to_mount;
int diskfd;

static struct fuse_operations ffs_operations = {
    .getattr    = ffs_getattr,
    //.readlink = ffs_readlink,
    .mknod    = ffs_mknod,
    .mkdir      = ffs_mkdir,
    .unlink     = ffs_unlink,
	.rmdir	    = ffs_rmdir,
	//.symlink	= ffs_symlink,
	.rename	    = ffs_rename,
	//.link	    = ffs_link,
	.chmod	    = ffs_chmod,
	.chown	    = ffs_chown,
	.truncate   = ffs_truncate,
	.open	    = ffs_open,
	.read	    = ffs_read,
	.write	    = ffs_write,
	//.statfs	    = ffs_statfs,
	.flush	    = ffs_flush,
	//.release	= ffs_release,
	//.fsync	    = ffs_fsync,
	//.setxattr	= ffs_setxattr,
	//.getxattr	= ffs_getxattr,
	//.listxattr	= ffs_listxattr,
	//.removexattr = ffs_removexattr,
	//.opendir	= ffs_opendir,
	.readdir	= ffs_readdir,/*
	//.releasedir	= ffs_releasedir,
	//.fsyncdir	= ffs_fsyncdir,
	.init	    = ffs_init,
	.destroy	= ffs_destroy,
	.access	    = ffs_access,
	.create	    = ffs_create,
	.ftruncate	= ffs_ftruncate,
	.fgetattr	= ffs_fgetattr,
	//.lock	    = ffs_lock,*/
	.utime	= ffs_utimens,
	/*.bmap	    = ffs_bmap,
	//.ioctl	    = ffs_ioctl,
	//.poll	    = ffs_poll,
	//.write_buf	= ffs_write_buf,
	//.read_buf	= ffs_read_buf,
	//.flock	    = ffs_flock,
	//.fallocate	= ffs_fallocate,*/
};

int main(int argc, char **argv) {
    diskfd = openDisk(argv[argc-1]);
    //init_fs();
	load_fs(diskfd);
    return fuse_main(argc-1, argv, &ffs_operations);
}
