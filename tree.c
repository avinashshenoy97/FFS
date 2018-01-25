#include "tree.h"


fs_tree_node *root, *global_curr;


/*
Initialize the tree structure that stores the file system's tree
*/
int init_fs() {
    //path_to_mount = (char *)malloc(sizeof(char) * (strlen(mountPoint) + 1));
    //strcpy(path_to_mount, mountPoint);

    root = (fs_tree_node *)malloc(sizeof(fs_tree_node));
    global_curr = (fs_tree_node *)malloc(sizeof(fs_tree_node));        //update this whenever CD is done
    printf("Root node at %p\n", root);

    root->type = 2;
    root->name = NULL;
    root->fullname = (char *)malloc(sizeof(char) * 2);
    strcpy(root->fullname, "/");

    root->children = NULL;
    root->len = 0;
    root->parent = NULL;
}

/*
Returns address of node if node exists in FS tree, else 0.
*/
fs_tree_node *node_exists(const char *path) {
    printf("node_exists called!\n");
    printf("Checking if : %s : exists\n", path);

    int s = 1, e = 0, l = strlen(path), sublen = 0;
    int i, j, found = 0;
    char *sub = NULL;
    fs_tree_node *curr = &root;
    
    if(!strcmp(path, "/")) {
        printf("node_exists returning with %p!\n", curr);
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
            printf("Length of part: %d\n", sublen);
            sub = (char *)malloc(sizeof(char) * sublen);
            sub[sublen - 1] = 0;

            for(i = s, j = 0 ; i < e ; i++, j++)
                sub[j] = path[i];

            printf("Part found : %s\n", sub);
            printf("Searching for part in %d children!\n", curr->len);

            for(i = 0 ; i < curr->len ; i++) {
                if(!strcmp(((curr->children)[i]).name, sub)) {
                    curr = curr->children + i;
                    printf("curr changed to %p\n", curr);
                    found = 1;
                    break;
                }
            }

            free(sub);
            if(!found) {
                printf("node_exists returning with 0!\n");
                return 0;
            }
        }
        else {
            break;
        }
        
        s = e + 1;

    }while(e != l);
    
    printf("node_exists returning with %p!\n", curr);
    return curr;
}

/*
Create a file at (path) of type specified by (mode)
If any intermediate directory in (path) doesn't exist, error is thrown
*/
int add_fs_tree_node(const char *path, short type) {
    fs_tree_node *curr = &root;
    int pathLength = strlen(path), sublen = 0;
    int i, j;
    char *temp = (char *)malloc(sizeof(char) * pathLength);     //to store path until one level higher than path given
    strcpy(temp, path);
    
    for(i = pathLength - 1 ; temp[i] != '/' ; i--);     //find first / from back of path
    temp[i] = 0;
    
    if(i == 0) {  //if root's child
        printf("Found to be root's child!\n");
        strcpy(temp, "/");
        i = 1;
    }

    sublen = (pathLength - i + 1);
    char *fileName = (char *)malloc(sizeof(char) * sublen);
    for(j = 0 ; i < pathLength ; i++, j++)                  //extract name of file from full path   
        fileName[j] = path[i];
    fileName[sublen - 1] = 0;

    printf("Name of file : %s\n", fileName);

    temp[i] = 0;
    printf("Checking if path : %s : exists\n", temp);

    if((curr = node_exists(temp))) {
        printf("Path found to exist with %d children!\n", curr->len);
        fs_tree_node *parent = curr;

        curr->len += 1;
        curr->children = realloc(curr->children, sizeof(fs_tree_node) * curr->len);
        curr = curr->children + (curr->len - 1);
        
        curr->name = (char *)malloc(sizeof(char) * sublen);     //add name to FS node
        strcpy(curr->name, fileName);

        curr->fullname = (char *)malloc(sizeof(char) * (pathLength + 1));       //add full name to FS node
        strcpy(curr->fullname, path);

        curr->children = NULL;

        curr->type = type;
        switch(type) {
            case 1:
                curr->len = -1;
                break;

            case 2:
                curr->len = 0;
                break;

            default:
                printf("TYPE error in add_fs_tree_node!\n");
        }

        curr->parent = parent;
    }

    printf("FS Node added at %p\n", curr);

    free(temp);
}