username = $(shell whoami)
mountpoint = /home/$(username)/Desktop/mountpoint
includepath = -I./include/
srcprefix = ./src/
files = $(srcprefix)ffs_operations.c $(srcprefix)tree.c $(srcprefix)disk.c $(srcprefix)bitmap.c
compileflags = -D_FILE_OFFSET_BITS=64
opflag = -o ffs
neededflag = `pkg-config fuse --libs` -DFUSE_USE_VERSION=22 -lm

all: run cleanup

run: compile cleanup
	./ffs -f $(mountpoint) /home/$(username)/Desktop/file.txt

drun: dcompile cleanup
	./ffs -d -f -s $(mountpoint) /home/$(username)/Desktop/file.txt

bgrun: compile
	./ffs $(mountpoint)

compile: checkdir
	gcc -Wall $(includepath) $(srcprefix)ffs_main.c $(files) $(compileflags) $(opflag) $(neededflag)

dcompile: checkdir
	gcc -Wall $(includepath) -g -DERR_FLAG $(srcprefix)ffs_main.c $(files) $(compileflags) $(opflag) $(neededflag)

checkdir:
	if [ -d "$(mountpoint)" ]; then echo "mountpoint exists"; else mkdir $(mountpoint); fi

dmkfs: mkfs_dcompile
	./mkfs /home/$(username)/Desktop/file.txt

mkfs_dcompile: 
	gcc -Wall $(includepath) -g -DERR_FLAG $(files) $(srcprefix)mkfs.c $(compileflags) -o mkfs $(neededflag)

mkfs: mkfs_compile
	./mkfs /home/$(username)/Desktop/file.txt

mkfs_compile: 
	gcc -Wall $(includepath) $(files) $(srcprefix)mkfs.c $(compileflags) -o mkfs $(neededflag)

cleanup :
	-fusermount -u $(mountpoint)
