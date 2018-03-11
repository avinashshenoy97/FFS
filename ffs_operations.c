#include "ffs_operations.h"
#include "tree.h"


// Error logging for THIS MODULE, helps differentiate from logging of other modules
// Prints errors and logging info to STDOUT
// Passes format strings and args to vprintf, basically a wrapper for printf
static void error_log(char *fmt, ...) {
#ifdef ERR_FLAG
    va_list args;
    va_start(args, fmt);
    
    printf("\n");
    printf("FfS OPS : ");
    vprintf(fmt, args);
    printf("\n");

    va_end(args);
#endif
}


int ffs_getattr(const char *path, struct stat *s) {
    error_log("%s called on path : %s", __func__, path);

    fs_tree_node *curr = NULL;
    if(!(curr = node_exists(path))) {
        error_log("curr = %p ; not found returning!", curr);
        return -ENOENT;
    }

    memset(s, 0, sizeof(struct stat));

    s->st_dev = 666;
    s->st_ino = curr->inode_no;

    switch(curr->type) {
        case 1:
            s->st_mode = S_IFREG | curr->perms;
            s->st_nlink = 1;
            break;

        case 2:
            s->st_mode = S_IFDIR | curr->perms;
            s->st_nlink = 2;
            break;

        default:
            error_log("Type not supported : %d", curr->type);
            return -ENOTSUP;
    }

    s->st_nlink += curr->len;
    s->st_uid = curr->uid;
    s->st_gid = curr->gid;

    s->st_size = curr->data_size;
    s->st_blocks = ((curr->data_size + NODE_SIZE) / 4096) + 1;
    s->st_blocks *= 8;

    s->st_atime = (curr->st_atim).tv_sec;
    s->st_mtime = (curr->st_mtim).tv_sec;
    s->st_ctime = (curr->st_ctim).tv_sec;
    
    return 0;
}


int ffs_mknod(const char *path, mode_t m, dev_t d) {
    error_log("%s called on path : %s", __func__, path);

    error_log("Add FS tree node at path : %s", path);
    long int ret = (uint64_t)add_fs_tree_node(path, 1);
    if(ret < 0) {
        return (int)ret;
    }

    return 0;
}


int ffs_mkdir(const char *path, mode_t m) {
    error_log("%s called on path : %s", __func__, path);

    error_log("Add FS tree node at path : %s", path);
    long int ret = (uint64_t)add_fs_tree_node(path, 2);
    if(ret < 0) {
        return (int)ret;
    }

    return 0;
}


int ffs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    error_log("%s called on path : %s", __func__, path);

    fs_tree_node *curr = NULL;

    filler(buffer, ".", NULL, 0);
    filler(buffer, "..", NULL, 0);

    curr = node_exists(path);       //check if it exists

    if(strcmp(path, "/")) {             //if its not root
        if(!curr) {
            return -ENOENT;
        }

        //curr = curr->parent;            //get link to its parent node
    }

    error_log("Path : %s : found to exist with %d children", path, curr->len);

    int i;
    for(i = 0 ; i < curr->len ; i++)
        filler(buffer, curr->children[i]->name, NULL, 0);

    return 0;
}


int ffs_rmdir(const char *path) {
    error_log("%s called on path : %s", __func__, path);
    if(node_exists(path)->len != 0) {
        //printf("rmdir: failed to remove '%s': Directory not empty", path);
        return -ENOTEMPTY;
    }
    // OS checks if path exists using getattr, no need to check explicitly
    // Just forward responsibility to tree.c function

    return remove_fs_tree_node(path);
}


int ffs_open(const char *path, struct fuse_file_info *fi)
{   
    error_log("%s called on path : %s", __func__, path);
    
    fs_tree_node *curr = node_exists(path);
    uint32_t check = 0;
    switch(fi->flags & O_ACCMODE) {
        case O_RDWR:
            error_log("O_RDWR");
            check = check | 0666;
            break;
        
        case O_RDONLY:
            error_log("O_RDONLY");
            check = check | 0444;
            break;

        case O_WRONLY:
            error_log("O_WRONLY");
            check = check | 0222;
            break;
    }

    error_log("%d %d %d", curr->perms, check, curr->perms & check);
    if(curr->perms & check) {
        error_log("Allowed to open!");
        return 0;
    }

    return -EACCES;
}


int ffs_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi)
{   
    error_log("%s called on path : %s \t size = %lu\t offset = %ld", __func__, path, size, offset);

    fs_tree_node *curr = NULL;
    size_t len;
    curr = node_exists(path);
    dataDiskReader(curr);
    error_log("Data disk read %s", curr->data);

    len = curr->data_size;

    error_log("curr found at %p with data %d", curr, len);

    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        
        error_log("if offset < len\t %ld %d %ld", size, len, offset);
        memcpy(buf, curr->data + offset, size);
    } 
    else {
        size = 0;
        strcpy(buf, "");
    }

    return size;
}


int ffs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{   
    error_log("%s called on path : %s ; to write : %s ; size = %d ; offset = %d ;", __func__, path, buf, size, offset);

    fs_tree_node *curr = NULL;
    size_t len = 0;
    curr = node_exists(path);
    len = curr->data_size;

    error_log("curr found at %p with data %d", curr, len);

    if (offset + size >= len){
        void *new_buf = NULL;

        new_buf = reallocate(curr, offset+size+1);
        if (!new_buf && offset+size) {
            error_log("Failed to realloc! %p && %d = %d", new_buf, offset+size, (!new_buf && offset+size));
            return -ENOMEM;
        }
        else if(new_buf != curr->data)
            curr->data = new_buf;

        error_log("successfuly realloced to %d!", offset+size+1);

        memset(curr->data + offset, 0, size);
        
        error_log("Erased data from offset %d to size %d!", offset, size);
        //curr->data = new_buf;
        curr->data_size = offset + size;
        error_log("curr->data_size %lu", curr->data_size);
    }
    
    memcpy(curr->data + offset, buf, size);

    error_log("Copied data! Returning with size %d!", size);

    ffs_flush(path, NULL);

    return size;
}


int ffs_utimens(const char *path, struct utimbuf *tv) {
    error_log("%s called on path : %s", __func__, path);
    error_log("atime = %s; mtime = %s ", ctime(&(tv->actime)), ctime(&(tv->modtime)));

    fs_tree_node *curr = node_exists(path);

    if(!curr)
        return -ENOENT;
    
    if(curr->st_atim.tv_sec < tv->actime)
        curr->st_atim.tv_sec = tv->actime;

    if(curr->st_mtim.tv_sec < tv->modtime)
        curr->st_mtim.tv_sec = tv->modtime;
    
    /*
    curr->st_atim.tv_nsec = tv[0].tv_nsec;
    curr->st_mtim.tv_nsec = tv[1].tv_nsec;
    */

    return 0;
}


int ffs_truncate(const char* path, off_t size)
{

 	error_log("%s called on path : %s ; to change to size = %d ;", __func__, path,size);

    fs_tree_node *curr = NULL;
    size_t len;
    curr = node_exists(path);
    len = curr->data_size;

    error_log("curr found at %p with data %d", curr, len);
    void *new_buf;
    
    if(len<size)      
    {  	 
    	 new_buf = reallocate(curr, size+1);
    	 
    	 if(!new_buf)
    	 {
      		return -ENOMEM;
    	 }
    	 
    	 //memcpy(new_buf, curr->data, len);
   		 //free(curr->data);
   		 //curr->data = new_data;
   	}
   	else if(len>size)
	{
	
		 new_buf = reallocate(curr, size+1);
    	 
    	 if(!new_buf)
    	 {
      		return -ENOMEM;
    	 }
	
   		//memcpy(new_buf, curr->data, size);
    	//free(curr->data);
   	    //curr->data = new_buf;
   	    
   	 } 	 
   	 
   	 // Fill remaining space with zeroes
 	 if(len<size)
 	 {
    	memset(curr->data + len, 0, size-len);
  	 }
  	 
 	 // Update file size
  	 curr->data_size = size;
  	 
  	 return 0;
    
}


int ffs_unlink(const char *path)
 {
  
  	error_log("%s called on path : %s ;", __func__, path);
    
    return remove_fs_tree_node(path);

}


int ffs_rename(const char *from, const char *to) {
    error_log("%s called from : %s ; to : %s", __func__, from, to);

    // check if destination exists
    fs_tree_node *to_node = node_exists(to);
    fs_tree_node *from_node = node_exists(from);
    fs_tree_node *from_parent;

    if(!from) {             // if from doesn't exist
        error_log("from file not found");
        return -ENOENT;
    }

    if(from_node->type == 1) {   // if from node is a file
        error_log("from node is a file");
        dataDiskReader(from_node);
        if(to_node) {   // if to node exists
            error_log("to node exists");
            if(to_node->type == 1) {    // if to node is also file
                error_log("to node is a file");

                remove_fs_tree_node(to);    // remove dest file
                error_log("to node was removed");
                to_node = add_fs_tree_node(to, 1);        // create new file at dest
                error_log("to node was added");
                copy_nodes(from_node, to_node);       // copy structs
                error_log("from node was copied to to node");

                // Now, remove from node without destroying from_node's members (since to is using its members)
                from_parent = from_node->parent;      // get parent of source node
                //if(!from_node->name)
                    //free(from_node->name);
                if(!from_node->fullname)
                    free(from_node->fullname);
                if(!from_node)
                    free(from_node);    // free the struct itself

                error_log("from node was freed");

                // Now, remove from_node from it's parent's children array
                int i;
                for(i = 0 ; i < from_parent->len ; i++) {
                    if(from_parent->children[i] == from_node) {
                        // when from_node is found in it's parents children array, move all other children ahead of it back, effectively removing it
                        error_log("from_node found to be the %d th child of its parent %p", i, from_parent);
                        int j;
                        for(j = i + 1 ; j < from_parent->len ; j++) {
                            from_parent->children[j-1] = from_parent->children[j];
                            from_parent->ch_inodes[j-1] = from_parent->ch_inodes[j];
                        }
                        from_parent->len -= 1;  //reduce child count by 1
                        error_log("from_parents children reduced from %d to %d", from_parent->len-1, from_parent->len);
                        break;  
                    }
                }
            }
            else if(to_node->type == 2) {   // if to node is a directory, from node must become child of the to_node
                // this block will probably never execute
                // if to path is a dir, OS or FUSE changes to path to dir/(from_file_name), i.e, same file name as from path but inside the folder specified in the (to) path
                error_log("to node is a dir, not yet implemented");
                
                return -EISDIR; // return "is a dir"
            }
        }
        else {  // if to node does not exist
            to_node = add_fs_tree_node(to, 1);        // create file at dest
            copy_nodes(from_node, to_node);    // copy from from to to

            // Now, remove from node without destroying from_node's members (since to is using its members)
            from_parent = from_node->parent;      // get parent of source node
            if(!from_node->name)
                free(from_node->name);
            if(!from_node->fullname)
                free(from_node->fullname);
            if(!from_node)
                free(from_node);    // free the struct itself
                
            error_log("from node was freed");

            // Now, remove from_node from it's parent's children array
            int i;
            for(i = 0 ; i < from_parent->len ; i++) {
                if(from_parent->children[i] == from_node) {
                    // when from_node is found in it's parents children array, move all other children ahead of it back, effectively removing it
                    error_log("from_node found to be the %d th child of its parent %p", i, from_parent);
                    int j;
                    for(j = i + 1 ; j < from_parent->len ; j++) {
                        from_parent->children[j-1] = from_parent->children[j];
                        from_parent->ch_inodes[j-1] = from_parent->ch_inodes[j];
                    }
                    from_parent->len -= 1;  //reduce child count by 1
                    error_log("from_parents children reduced from %d to %d", from_parent->len-1, from_parent->len);
                    break;  
                }
            }
        }
    }
    else {
        // if from_node is a directory
        error_log("from node must be a dir");

        if(to_node) {   // if it exists
            error_log("to node exists");

            if(to_node->type == 1) {     // check if its a file
                error_log("to node is a file");
                // this block will probably never execute
                // if to path is a file and from is a dir, OS or FUSE will refuse automatically

                return -EEXIST;     //if it is a file, return "File already exists" error like Ubuntu does
            }
            else {
                error_log("to node is a dir, deleting");
                remove_fs_tree_node(to);
                error_log("Deleted!");
            }
        }
        else {      // if it doesn't exist
            error_log("to node does not exist");
        }

        to_node = add_fs_tree_node(to, 2);    // create the directory
        error_log("Added node");
        copy_nodes(from_node, to_node);
        error_log("Copied from to to");

        // Now, remove from node without destroying from_node's members (since to is using its members)
        from_parent = from_node->parent;      // get parent of source node
        if(!from_node->parent)
            free(from_node->name);
        if(!from_node->fullname)
            free(from_node->fullname);
        if(!from_node)
            free(from_node);    // free the struct itself

        error_log("from node was freed");

        // Now, remove from_node from it's parent's children array
        int i;
        for(i = 0 ; i < from_parent->len ; i++) {
            if(from_parent->children[i] == from_node) {
                // when from_node is found in it's parents children array, move all other children ahead of it back, effectively removing it
                error_log("from_node found to be the %d th child of its parent %p", i, from_parent);
                int j;
                for(j = i + 1 ; j < from_parent->len ; j++) {
                    from_parent->children[j-1] = from_parent->children[j];
                    from_parent->ch_inodes[j-1] = from_parent->ch_inodes[j];
                }
                from_parent->len -= 1;  //reduce child count by 1
                from_parent->children = realloc(from_parent->children, sizeof(fs_tree_node *) * from_parent->len);
                from_parent->ch_inodes = realloc(from_parent->ch_inodes, sizeof(from_parent->inode_no) * from_parent->len);
                error_log("from_parents children reduced from %d to %d", from_parent->len+1, from_parent->len);
                break;  
            }
        }
    }

    void *buf;
    uint64_t newblocks = constructBlock(from_parent, &buf);
    diskWriter(buf, newblocks, from_parent->inode_no);
    if(!buf)
        free(buf);

    newblocks = constructBlock(to_node, &buf);
    diskWriter(buf, newblocks, to_node->inode_no);
    if(!buf)
        free(buf);

    /*
    newblocks = constructBlock(to_node->parent, &buf);
    diskWriter(buf, newblocks, to_node->parent->inode_no);
    if(!buf)
        free(buf);*/

    

    error_log("end of %s reached, going to return 0", __func__);

    return 0;
}


int ffs_chmod(const char *path, mode_t setPerm) {
    error_log("%s called on path : %s ; to set : %d", __func__, path, setPerm);

    fs_tree_node *curr = node_exists(path);
    if(!curr) {
        error_log("File not found!");
        
        return -ENOENT;
    }

    uint32_t curr_uid = getuid();
    if(curr_uid == curr->uid || !curr_uid) {        // if owner is doing chmod or root is
        error_log("Current user (%d) has permissions to chmod", curr_uid);

        curr->perms = setPerm;
    }
    else {
        error_log("Current user (%d) DOESNT permissions to chown", curr_uid); 
        return -EACCES;
    }

    return 0;
}


int ffs_chown(const char *path, uid_t u, gid_t g) {
    error_log("%s called on path : %s ; to set : u = %d\t g = %d", __func__, path, u, g);

    fs_tree_node *curr = node_exists(path);
    if(!curr) {
        error_log("File not found!");
        
        return -ENOENT;
    }

    uid_t curr_user = getuid();

    if(curr_user != 0 && curr_user != curr->uid) {       // only root or owner can chown a file
        error_log("Current user (%d) DOESNT permissions to chown file owned by %d", curr_user, curr->uid); 
        return -EACCES;
    }
    error_log("Current user (%d) has permissions to chown", curr_user); 

    if(u != -1)
        curr->uid = u;

    if(g != -1)
        curr->gid = g;

    return 0;
}


int ffs_flush(const  char *path, struct fuse_file_info *fi) {
    error_log("%s called on path : %s", __func__, path);

    fs_tree_node *node = node_exists(path);
    void *buf = NULL;
    uint64_t blocks = constructBlock(node, &buf);
    diskWriter(buf, blocks, node->inode_no);
    error_log("Wrote file!");
    //free(buf);    // free(): invalid pointer = error thrown, unknown reason

    return 0;
}