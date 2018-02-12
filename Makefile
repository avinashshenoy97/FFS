username = $(shell whoami)
files = ffs_operations.c tree.c disk.c bitmap.c
compileflags = -D_FILE_OFFSET_BITS=64
opflag = -o ffs
neededflag = `pkg-config fuse --libs` -DFUSE_USE_VERSION=22 -lm

all: run

run: compile	
	./ffs -f /home/$(username)/Desktop/mountpoint /home/$(username)/Desktop/file.txt

drun: dcompile
	./ffs -d -f -s /home/$(username)/Desktop/mountpoint /home/$(username)/Desktop/file.txt

bgrun: compile
	./ffs /home/$(username)/Desktop/mountpoint

compile: checkdir
	gcc -Wall ffs_main.c $(files) $(compileflags) $(opflag) $(neededflag)

dcompile: checkdir
	gcc -Wall -g -DERR_FLAG ffs_main.c $(files) $(compileflags) $(opflag) $(neededflag)

checkdir:
	if [ -d "/home/$(username)/Desktop/mountpoint" ]; then echo "mountpoint exists"; else mkdir /home/$(username)/Desktop/mountpoint; fi

dmkfs: mkfs_dcompile
	./mkfs /home/$(username)/Desktop/file.txt

mkfs_dcompile: 
	gcc -Wall -g -DERR_FLAG $(files) mkfs.c $(compileflags) -o mkfs $(neededflag)

mkfs: mkfs_compile
	./mkfs /home/$(username)/Desktop/file.txt

mkfs_compile: 
	gcc -Wall $(files) mkfs.c $(compileflags) -o mkfs $(neededflag)