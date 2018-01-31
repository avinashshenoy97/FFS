#include "tree.h"

// Root
fs_tree_node *root;

// Error logging for THIS MODULE, helps differentiate from logging of other modules
// Prints errors and logging info to STDOUT
// Passes format strings and args to vprintf, basically a wrapper for printf
static void error_log(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    printf("TREE : ");
    vprintf(fmt, args);
    printf("\n");

    va_end(args);
}

/*
Free all dynamically allocated members of node
Free node itself too
return 0 if all okay, else return -1
*/
int destroy_node(fs_tree_node *node) {
    error_log("%s called on %p", __func__, node);

    free(node->name);
    free(node->fullname);
    
    if(node->children != NULL)
        free(node->children);
    
    node->parent = NULL;

    if(node->data != NULL)
        deallocate(node);
    
    free(node);
    return 0;
}

/*
Initialize the tree structure that stores the file system's tree
*/
int init_fs() {
    error_log("%s called", __func__);
    //path_to_mount = (char *)malloc(sizeof(char) * (strlen(mountPoint) + 1));
    //strcpy(path_to_mount, mountPoint);

    root = (fs_tree_node *)malloc(sizeof(fs_tree_node));
    //global_curr = (fs_tree_node *)malloc(sizeof(fs_tree_node));        //update this whenever CD is done
    error_log("Root node at %p", root);

    root->type = 2;
    root->name = NULL;
    root->fullname = (char *)malloc(sizeof(char) * 2);
    strcpy(root->fullname, "/");

    root->children = NULL;
    root->len = 0;
    root->parent = NULL;
    root->data = NULL;
    root->data_size = 0;
    root->block_count = 0;

    return 0;
}

/*
Uses Depth-First-Search to recursively apply the function (foo) to each node under (curr) and to (curr) itself.
*/
int dfs_dispatch(fs_tree_node *curr, int (*foo)(fs_tree_node *)) {
    error_log("%s called on node %s", __func__, curr->fullname);

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

/*
Returns address of node if node exists in FS tree, else 0.
*/
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

/*
Create a file at (path) of type specified by (mode)
If any intermediate directory in (path) doesn't exist, error is thrown
Returns address of added node
*/
fs_tree_node *add_fs_tree_node(const char *path, short type) {
    error_log("%s called! path = %s \t type=%d", __func__, path, type);

    fs_tree_node *curr = root;
    int pathLength = strlen(path), sublen = 0;
    int i, j;
    char *temp = (char *)malloc(sizeof(char) * pathLength);     //to store path until one level higher than path given
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
        error_log("Path found to exist with %d children!", curr->len);
        fs_tree_node *parent = curr;

        curr->len += 1;
        curr->children = realloc(curr->children, sizeof(fs_tree_node *) * curr->len);
        curr->children[curr->len - 1] = (fs_tree_node *)malloc(sizeof(fs_tree_node));
        curr = curr->children[curr->len - 1];
        
        curr->name = (char *)malloc(sizeof(char) * sublen);     //add name to FS node
        strcpy(curr->name, fileName);

        curr->fullname = (char *)malloc(sizeof(char) * (pathLength + 1));       //add full name to FS node
        strcpy(curr->fullname, path);

        curr->children = NULL;

        curr->type = type;
        curr->len = 0;
        curr->parent = parent;
    }

    curr->data = NULL;
    curr->data_size = 0;
    curr->block_count = 0;

    time(&(curr->st_ctim).tv_sec);
    curr->st_mtim = curr->st_atim = curr->st_ctim;

    error_log("FS Node added at %p", curr);
    free(temp);
    return curr;
}

/*
Remove node at (path)
If any intermediate directory in (path) doesn't exist, error is thrown
*/
int remove_fs_tree_node(const char *path) {
    error_log("%s called with path %s", __func__, path);

    if(!strcmp(path, "/")) {    //if root, return -1 (operation not permitted)
        error_log("%s Returning with -1", __func__);
        return -1;
    }

    // OS checks if path exists using getattr, no need to check explicitly
    // using node_exists to get FS tree node

    int i;
    fs_tree_node *toDelete = node_exists(path);
    fs_tree_node *parent = toDelete->parent;

    error_log("Deleting node at %p, child of %p", toDelete, parent);

    dfs_dispatch(toDelete, &destroy_node);

    for(i = 0 ; i < parent->len ; i++) {
        if(parent->children[i] == toDelete) {
            error_log("%p found to be %d th child of %p", toDelete, i, parent);
            break;
        }
    }

    for( ; i < --(parent->len) ; i++)                   // shift all children back one position, effectively deleting the node
        parent->children[i] = parent->children[i+1];

    return 0;
}

/*
Copies all members from (from) to (to), except link to parent, name and fullname
Assumes both nodes already exist and are allocated space, but pointer members of (to) are not allocated
Does not free anything, strictly copy
Returns 0
*/
int copy_nodes(fs_tree_node *from, fs_tree_node *to) {
    to->type = from->type;                       //type of node
    //to->name = from->name;                         //name of node
    //to->fullname = from->fullname;                     //full path of node

    //to->parent = from->parent;        //link to parent
    to->children = from->children;      //links to children
    to->len = from->len;                       //number of children

    to->data = from->data;						//data for read and write
    to->data_size = from->data_size;						//size of data
    to->block_count = from->block_count;               // number of blocks

    to->st_atim = from->st_atim;            /* time of last access */
    to->st_mtim = from->st_mtim;            /* time of last modification */
    to->st_ctim = from->st_ctim;            /* time of last status change */

    return 0;
}