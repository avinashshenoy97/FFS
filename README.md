# FFS

Custom file system with FUSE.

## Instructions

Clone the repo

    git clone https://github.com/avinashshenoy97/ffs ffs

`cd` into the repo

    cd ffs

Edit the Makefile and replace the username with your username

    username = <your username here>

Start FFS with default mountpoint by typing `make` inside the `ffs` folder. You can start it with a different mount point by running

    ./ffs -f <path to mount point>

To run as background daemon,

    ./ffs <path to mount point>

The mountpoint is a folder created on your Desktop. You can `cd` into `~/Desktop/mountpoint` and test the file system.

Use this command to unmount FFS (remember to cd out of it first!)

    fusermount -u ~/Desktop/mountpoint