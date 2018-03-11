# <img src="https://github.com/avinashshenoy97/FFS/blob/master/extras/icon.png" width="10%"> FFS

[![GitHub issues](https://img.shields.io/github/issues/avinashshenoy97/FFS.svg?style=flat)](https://github.com/avinashshenoy97/FFS/issues)
[![Open Source Love](https://badges.frapsoft.com/os/v1/open-source.svg?v=103)]()
[![License](https://img.shields.io/badge/license-mit-brightgreen.svg?style=flat)](https://github.com/avinashshenoy97/FFS/blob/master/LICENSE)

[![forthebadge](https://forthebadge.com/images/badges/60-percent-of-the-time-works-every-time.svg)](https://forthebadge.com)
[![forthebadge](https://forthebadge.com/images/badges/compatibility-betamax.svg)](https://forthebadge.com)



FFS is a custom, persistent, virtual file system built with FUSE. Virtual file systems run on top of an already existing, more robust, and more reliable file system. The virtual file system is responsbile for maintaining the file system tree structure. Most virtual file systems built with FUSE usually do not implement any level of persistence, however, FFS does. FFS uses a file, at `~/Desktop/file.txt` by default, to store data across mounts and machine reboots. FFS also requires a mountpoint through which normal FS operations can be done. By default, it is `~/Desktop/mountpoint`. 

Ideally, once mounted, normal Linux operations such as create, open, read, write, and so on, can be done for files stored inside the mountpoint as if it were any other directory.

## Running FFS

Clone the repo

    git clone https://github.com/avinashshenoy97/ffs ffs

`cd` into the repo

    cd ffs

Edit the Makefile and replace the username with your username

    username = <your username here>

Compile and run our version of `mkfs` to initialise the file system by running

    make mkfs

Start FFS with default mountpoint by typing `make` inside the `ffs` folder.

---

To use a different mountpoint and persistent disk file, first compile and run our MKFS 

    gcc -Wall ffs_operations.c tree.c disk.c bitmap.c mkfs.c -D_FILE_OFFSET_BITS=64 -o mkfs `pkg-config fuse --libs` -DFUSE_USE_VERSION=22 -lm

    ./mkfs <path_to_persistent_storage>

Then compile and run FFS

    gcc -Wall ffs_main.c ffs_operations.c tree.c disk.c bitmap.c -D_FILE_OFFSET_BITS=64 -o ffs `pkg-config fuse --libs` -DFUSE_USE_VERSION=22 -lm

    ./ffs -f <path to mount point>

To run as background daemon,

    ./ffs <path to mount point>

---

### Flags to `./ffs`

There are several flags that can be passed to the `./ffs` to tweak how FUSE will run FFS.

| FLAG | MEANING       | CONSEQUENCE |
|:----:|:-------------:|:-----------:|
|-d|Debug mode| Additional debugging information is printed by FUSE|
|-f|Run in foreground| Without this flag, FFS will be run as a background daemon|
|-s| Single threaded | FFS runs in single threaded mode. Without this flag, FFS would run on multiple threads, possibly making it faster.|

---

### Flags to `gcc`

Several flags are passed to `gcc` while compiling FFS. Here is a table to understand their meaning.

| FLAG | MEANING       | CONSEQUENCE |
|:----:|:-------------:|:-----------:|
|-Wall|Turn on warnings| Display all warnings|
|-g|Add debugging symbols|The program is compiled with debugging symbols and other information that can be used by tools like `gdb` for debugging.|
|-o|Output executable| Used to set the name of the compiled executable.|
|-D_FILE_OFFSET_BITS=64|Required by FUSE|This is a flag required by this version of FUSE.|
|\`pkg-config fuse --libs\` -DFUSE_USE_VERSION=22|Required|These flags are required to use the correct version of FUSE, the same version used to develop FFS.|
|-lm|Link math library|Used to link the math library for functions like `pow`|
|-DERR_FLAG|Error logging file|A flag used by FFS to enable/disable helpful debugging info while FFS runs. (FFS must run in the foreground to view these messages).|

---

The mountpoint, by default, is a folder created on the Desktop. You can `cd` into `~/Desktop/mountpoint` and test the file system by running common commands like

    mkdir a
    cd a
    mkdir b
    cd b
    pwd
    echo "Abc" > c.txt
    cd ..
    ls
    ls -a
    cd ..
    tree

The output should look like
![FFS Test Output](https://github.com/avinashshenoy97/FFS/blob/master/extras/output.png "Output")


Use this command to unmount FFS (remember to cd out of it first!)

    fusermount -u ~/Desktop/mountpoint


## Debug Mode

FFS can be compiled and run in debug mode by

    make drun

To just compile in debug mode

    make dcompile

Again, the default mountpoint and file path are used. To specify the path to mountpoint and file manually, 

    gcc -Wall -g -DERR_FLAG ffs_main.c ffs_operations.c tree.c disk.c bitmap.c -D_FILE_OFFSET_BITS=64 -o ffs `pkg-config fuse --libs` -DFUSE_USE_VERSION=22 -lm

    ./ffs -d -f -s <path to mount point>

---

## Pimary Contributors

|   |   |
|:-:|:-:|
| <img src="https://github.com/AnvithShetty10.png" width="75"> | [Anvith Shetty](https://github.com/AnvithShetty10) |
| <img src="https://github.com/anwika97.png" width="75"> | [Anwika Bhandary](https://github.com/anwika97) |
| <img src="https://github.com/avinashshenoy97.png" width="75"> | [Avinash Shenoy](https://github.com/avinashshenoy97) |
