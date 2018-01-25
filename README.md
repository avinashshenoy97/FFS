# FFS

Custom file system with FUSE.

## Instructions

Clone the repo

    git clone https://github.com/avinashshenoy97/ffs ffs

`cd` into the repo

    cd ffs

Edit the Makefile and replace the username with your username

    username = <your username here>

The mountpoint is a folder created on your Desktop. You can `cd` into `~/Desktop/mountpoint` and test the file system.

Use this command to unmount FFS (remember to cd out of it first!)

    fusermount -u ~/Desktop/mountpoint