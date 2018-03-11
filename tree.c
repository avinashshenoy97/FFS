#include "tree.h"

// Root
fs_tree_node *root;

// Error logging for THIS MODULE, helps differentiate from logging of other modules
// Prints errors and logging info to STDOUT
// Passes format strings and args to vprintf, basically a wrapper for printf
static void error_log(char *fmt, ...) {
#ifdef ERR_FLAG
    va_list args;
    va_start(args, fmt);
    
    printf("TREE : ");
    vprintf(fmt, args);
    printf("\n");

    va_end(args);
#endif
}


int destroy_node(fs_tree_node *node) {
    error_log("%s called on %p", __func__, node);

    if(!node->name)
        free(node->name);
    error_log("Erased name");
    if(!node->fullname)
        free(node->fullname);
    error_log("Erased fullname");
    
    if(node->children != NULL)
        free(node->children);
    error_log("Erased children");
    
    node->parent = NULL;
    error_log("Erased parent");

    if(node->data != NULL)
        deallocate(node);
    error_log("Erased data");
    
    //free(node);   // causes double free error
    error_log("Returning");
    return 0;
}


void output_node(fs_tree_node node) {
    error_log("Type : %d\nName : %s\nfullname : %s\nuid : %d\ngid : %d\nperms : %d\nnlinks : %d\nparent : %p\nchildren : %p\nlen : %u\ndata : %p\ndata_size : %lu\nblock_count : %lu\ninode_no : %lu\nst_atim : %s\nst_mtim : %s\nst_ctim : %s\n", node.type, node.name, node.fullname, node.uid, node.gid, node.perms, node.nlinks, node.parent, node.children, node.len, node.data, node.data_size, node.block_count, node.inode_no, ctime(&node.st_atim.tv_sec), ctime(&node.st_mtim.tv_sec), ctime(&node.st_ctim.tv_sec));
}


int init_fs() {
    error_log("%s called", __func__);
    //path_to_mount = (char *)malloc(sizeof(char) * (strlen(mountPoint) + 1));
    //strcpy(path_to_mount, mountPoint);

    root = (fs_tree_node *)malloc(sizeof(fs_tree_node));
    //global_curr = (fs_tree_node *)malloc(sizeof(fs_tree_node));        //update this whenever CD is done
    error_log("Root node at %p", root);

    root->type = 2;
    //root->name = NULL;
    root->fullname = (char *)malloc(sizeof(char) * 2);
    strcpy(root->fullname, "/");
    strcpy(root->name, "/");

    root->children = NULL;
    root->ch_inodes = NULL;
    root->len = 0;
    root->nlinks = 2;
    root->parent = NULL;
    root->data = NULL;
    root->data_size = 0;
    root->block_count = 0;

    return 0;
}


int dfs_dispatch(fs_tree_node *curr, int (*foo)(fs_tree_node *)) {
    error_log("%s called on node %s, len = %u, type = %d", __func__, curr->name, curr->len, curr->type);

    int i = 0;
    if(curr->len > 0 && curr->type == 2) {         // if curr has children and is directory
        error_log("Has %d children, curr->children is %p", curr->len, curr->children);
        for(i = 0 ; i < curr->len ; i++)        // call dsf_dispatch on each child
            if(curr->children[i]->type == 2)                    //if it is a directory, only dirs can have children
                dfs_dispatch(curr->children[i], foo);
    }

    // when a node with no children is found
    // apply foo to it

    foo(curr);
    return 0;
}


int bfs_dispatch(fs_tree_node *curr, int (*foo)(fs_tree_node *)) {
    error_log("%s called on node %s", __func__, curr->fullname);

    int i = 0;
    if(curr->len > 0 && curr->type == 2) {         // if curr has children and is directory
        error_log("Has %d children, curr->children is %p", curr->len, curr->children);
        for(i = 0 ; i < curr->len ; i++)        // call foo on each child
            foo(curr->children[i]);

        for(i = 0 ; i < curr->len ; i++)        // call bsf_dispatch on each child
            bfs_dispatch(curr->children[i], foo);
    }

    return 0;
}


fs_tree_node *node_exists(const char *path) {
    error_log("%s called!", __func__);
    error_log("Checking if : %s : exists", path);

    int s = 1, e = 0, l = strlen(path), sublen = 0;
    int i, j, found = 0;
    char *sub = NULL;
    fs_tree_node *curr = root;
    
    if(!strcmp(path, "/")) {
        error_log("%s returning with %p!", __func__, curr);
        return curr;
    }

    do {
        found = 0;
        for(i = s ; i < l ; i++) {
            if(path[i] == '/') {
                e = i;
                break;
            }
        }

        if(i == l)
            e = l;

        if(path[s] == '/')
            s += 1;

        sublen = e - s + 1;
        if(sublen > 0) {
            error_log("Length of part: %d", sublen);
            sub = (char *)malloc(sizeof(char) * sublen);
            sub[sublen - 1] = 0;

            for(i = s, j = 0 ; i < e ; i++, j++)
                sub[j] = path[i];

            error_log("Part found : %s", sub);
            error_log("Searching for part in %d children!", curr->len);

            for(i = 0 ; i < curr->len ; i++) {
                if(!strcmp(((curr->children)[i])->name, sub)) {
                    curr = curr->children[i];
                    error_log("curr changed to %p", curr);
                    found = 1;
                    break;
                }
            }
            error_log("Done searching");
            if(!sub)
                free(sub);
            if(!found) {
                error_log("%s returning with 0 not found!", __func__);
                return 0;
            }
        }
        else {
            break;
        }
        
        s = e + 1;

    }while(e != l);
    
    error_log("%s returning with %p!", __func__, curr);
    return curr;
}


fs_tree_node *add_fs_tree_node(const char *path, uint8_t type) {
    error_log("%s called! path = %s \t type=%d", __func__, path, type);

    fs_tree_node *curr = root;
    int pathLength = strlen(path), sublen = 0;
    int i, j;
    char *temp = (char *)malloc(sizeof(char) * (pathLength + 1));     //to store path until one level higher than path given
    strcpy(temp, path);
    
    for(i = pathLength - 1 ; temp[i] != '/' ; i--);     //find first / from back of path
    temp[i] = 0;
    i += 1;

    if(i == 1) {  //if root's child
        error_log("Found to be root's child!");
        strcpy(temp, "/");
    }

    sublen = (pathLength - i + 1);
    char *fileName = (char *)malloc(sizeof(char) * sublen);
    for(j = 0 ; i < pathLength ; i++, j++)                  //extract name of file from full path   
        fileName[j] = path[i];
    fileName[sublen - 1] = 0;

    error_log("Name of file : %s", fileName);

    temp[i] = 0;
    error_log("Checking if path : %s : exists", temp);

    if((curr = node_exists(temp))) {
        // FUSE checks for entire path to exist (and makes sure it will exist when this called)
        // Hence this block will usually be executed

        error_log("Path found to exist with %d children!", curr->len);
        fs_tree_node *parent = curr;

        curr->len += 1;
        curr->children = realloc(curr->children, sizeof(fs_tree_node *) * curr->len);
        curr->children[curr->len - 1] = (fs_tree_node *)malloc(sizeof(fs_tree_node));
        curr = curr->children[curr->len - 1];

        //now curr is child

        curr->inode_no = findFirstFreeBlock();
        if(curr->inode_no == -1) {
            error_log("Returning with error ENOSPC");
            return (fs_tree_node *)(-ENOSPC);
        }
        
        //curr->name = (char *)malloc(sizeof(char) * sublen);     //add name to FS node
        strcpy(curr->name, fileName);

        curr->fullname = (char *)malloc(sizeof(char) * (pathLength + 1));       //add full name to FS node
        strcpy(curr->fullname, path);

        curr->children = NULL;
        curr->ch_inodes = NULL;

        curr->type = type;
        curr->len = 0;
        curr->parent = parent;

        void *temp = realloc(parent->ch_inodes, parent->len * sizeof(parent->inode_no));
        if(!temp) {
            error_log("Error reallocing inode children array of parent");
            return (fs_tree_node *)(-ENOMEM);
        }
        parent->ch_inodes = temp;
        (parent->ch_inodes)[parent->len - 1] = curr->inode_no;
        error_log("(parent->ch_inodes)[parent->len - 1] = %lu", (parent->ch_inodes)[parent->len - 1]);
    }

    curr->uid = getuid();
    curr->gid = getgid();

    curr->data = NULL;
    curr->data_size = 0;
    curr->block_count = 0;

    time(&(curr->st_ctim).tv_sec);
    curr->st_mtim = curr->st_atim = curr->st_ctim;

    switch(type) {
        case 1:
            curr->perms = DEF_FILE_PERM;
            curr->nlinks = 1;
            break;

        case 2:
            curr->perms = DEF_DIR_PERM;
            curr->parent->nlinks += 1;
            curr->nlinks = 2;
            break;
    }

    error_log("FS Node added at %p", curr);
    if(!temp)
        free(temp);
    if(!fileName)
        free(fileName);

    
    error_log("Going to write to disk");
    void *buf = NULL;
    error_log("buf = %p", buf);
    int blocks_to_write = constructBlock(curr, &buf);
    error_log("Constructed block, bug = %p", buf);
    diskWriter(buf, blocks_to_write, curr->inode_no);
    error_log("Wrote to disk buf = %p", buf);
    saveBitMap();

    //free(buf); // throws error "invalid pointer" = reason unkown
    
    error_log("Starting on parent");
    blocks_to_write = constructBlock(curr->parent, &buf);
    error_log("Constructed parent block");
    diskWriter(buf, blocks_to_write, curr->parent->inode_no);
    error_log("Rewrote parent to disk");

    return curr;
}


int remove_fs_tree_node(const char *path) {
    error_log("%s called with path %s", __func__, path);

    if(!strcmp(path, "/")) {    //if root, return -1 (operation not permitted)
        error_log("%s Returning with -1", __func__);
        return -1;
    }

    // OS checks if path exists using getattr, no need to check explicitly
    // using node_exists to get FS tree node

    uint64_t i;
    fs_tree_node *toDelete = node_exists(path);
    fs_tree_node *parent = toDelete->parent;

    error_log("Deleting node at %p, child of %p", toDelete, parent);

    uint64_t next = toDelete->inode_no;
    dfs_dispatch(toDelete, &destroy_node);

    for(i = 0 ; i < parent->len ; i++) {
        if(parent->children[i] == toDelete) {
            error_log("%p found to be %d th child of %p", toDelete, i, parent);
            break;
        }
    }

    for( ; i < (parent->len - 1) ; i++) {                   // shift all children back one position, effectively deleting the node
        parent->children[i] = parent->children[i+1];
        parent->ch_inodes[i] = parent->ch_inodes[i+1];
    }

    parent->children = realloc(parent->children, sizeof(fs_tree_node *) * (parent->len - 1));
    parent->ch_inodes = realloc(parent->ch_inodes, sizeof(parent->inode_no) * (parent->len - 1));
    
    --(parent->len);

    void *buf = malloc(BLOCK_SIZE);
    error_log("Disk clear : next = %lu", next);
    while(next) {
        clearBitofMap(next);
        readBlock(next, buf);
        memcpy(&next, buf + BLOCK_SIZE - sizeof(next), sizeof(next));
        error_log("NEXT = %lu", next);
    }
    error_log("Done");
    if(!buf)
        free(buf);

    error_log("Rewriting parent now");
    void *buf2;
    uint64_t ret = constructBlock(parent, &buf2);
    diskWriter(buf2, ret, parent->inode_no);

    error_log("Returning with 0");
    return 0;
}


int copy_nodes(fs_tree_node *from, fs_tree_node *to) {
    error_log("%s called", __func__);
    to->type = from->type;                       //type of node
    //strcpy(to->name, from->name);                         //name of node
    //from->name = NULL;
    //to->fullname = from->fullname;                     //full path of node
    //to->inode_no = from->inode_no;

    //to->parent = from->parent;        //link to parent
    to->children = from->children;      //links to children
    to->ch_inodes = from->ch_inodes;
    //to->inode_no = from->inode_no;
    to->len = from->len;                       //number of children

    to->data = from->data;						//data for read and write
    to->data_size = from->data_size;						//size of data
    error_log("COPYING DATA %d : %10s : %10s", to->data_size, to->data, from->data);
    to->block_count = from->block_count;               // number of blocks

    to->st_atim = from->st_atim;            /* time of last access */
    to->st_mtim = from->st_mtim;            /* time of last modification */
    to->st_ctim = from->st_ctim;            /* time of last status change */

    return 0;
}


int load_fs(int diskfd) {
    error_log("%s called with diskfd %d", __func__, diskfd);
    uint64_t size;
    lseek(diskfd, 0, SEEK_SET);
    read(diskfd, &size, sizeof(size));
    error_log("Size of disk : %lu", size);

    loadBitMap(diskfd);
    print_bitmap();

    uint64_t toRead = ((bmap_size / BLOCK_SIZE + 1) + SUPERBLOCKS);
    error_log("toRead = %d", toRead);

    // Load root node
    root = diskReader(toRead);
    error_log("Root node at %p", root);
    error_log("With children %u", root->len);
    
    root->fullname = NULL;
    root->parent = NULL;
    root->children = NULL;
    root->data = NULL;
    root->block_count = 0;

    output_node(*root);

    fill_fs_tree(root);

    error_log("Done loading");
    return 0;
}


void fill_fs_tree(fs_tree_node *root) {
    error_log("%s called with root %p with name %s", __func__, root, root->name);
    uint64_t i;
    root->fullname = NULL;
    root->children = NULL;
    root->data = NULL;
    root->block_count = 0;

    root->children = (fs_tree_node **)malloc(sizeof(fs_tree_node *) * root->len);

    for(i = 0 ; i < root->len ; i++) {
        root->children[i] = diskReader(root->ch_inodes[i]);
        root->children[i]->parent = root;
    }

    for(i = 0 ; i < root->len ; i++) {
        fill_fs_tree(root->children[i]);
    }
    
    return;
}