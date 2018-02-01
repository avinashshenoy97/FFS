username = $(shell whoami)
files = ffs_main.c ffs_operations.c tree.c disk.c
debug_flag_file = derrlog.c
nodebug_flag_file = errlog.c
compileflags = -D_FILE_OFFSET_BITS=64
opflag = -o ffs
neededflag = `pkg-config fuse --libs` -DFUSE_USE_VERSION=22

all: run

run: compile	
	./ffs -f /home/$(username)/Desktop/mountpoint

drun: dcompile
	./ffs -d -f -s /home/$(username)/Desktop/mountpoint 

compile: checkdir
	gcc -Wall $(files) $(nodebug_flag_file) $(compileflags) $(opflag) $(neededflag)

dcompile: checkdir
	gcc -Wall -g $(files) $(debug_flag_file) $(compileflags) $(opflag) $(neededflag)	

checkdir:
	if [ -d "/home/$(username)/Desktop/mountpoint" ]; then echo "mountpoint exists"; else mkdir /home/$(username)/Desktop/mountpoint; fi
