username = ubuntu
files = ffs_main.c ffs_operations.c tree.c
compileflags = -D_FILE_OFFSET_BITS=64
opflag = -o ffs
neededflag = `pkg-config fuse --libs` -DFUSE_USE_VERSION=22

all: run

run: compile	
	./ffs -f /home/$(username)/Desktop/mountpoint

debugrun: dcompile
	./ffs -d -f -s /home/$(username)/Desktop/mountpoint

compile: checkdir
	gcc -Wall $(files) $(compileflags) $(opflag) $(neededflag)

dcompile: checkdir
	gcc -Wall -g $(files) $(compileflags) $(opflag) $(neededflag)	

checkdir:
	if [ -d "/home/$(username)/Desktop/mountpoint" ]; then echo "mountpoint exists"; else mkdir /home/$(username)/Desktop/mountpoint; fi