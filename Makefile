username = ubuntu
files = ffs_main.c ffs_operations.c tree.c
compileflags = -D_FILE_OFFSET_BITS=64
opflag = -o ffs
neededflag = `pkg-config fuse --libs` -DFUSE_USE_VERSION=22

all: checkdir

run: compile	
	./ffs -f /home/$(username)/Desktop/mountpoint

debugrun: checkdir dcompile
	if [ -d "/home/$(username)/Desktop/mountpoint" ]; then echo "mountpoint exists"; else mkdir /home/$(username)/Desktop/mountpoint; fi
	./ffs -d -f -s /home/$(username)/Desktop/mountpoint

checkdir: run
	if [ -d "/home/$(username)/Desktop/mountpoint" ]; then echo "mountpoint exists"; else mkdir /home/$(username)/Desktop/mountpoint; fi

compile: 
	gcc -Wall $(files) $(compileflags) $(opflag) $(neededflag)

dcompile: 
	gcc -Wall -g $(files) $(compileflags) $(opflag) $(neededflag)	