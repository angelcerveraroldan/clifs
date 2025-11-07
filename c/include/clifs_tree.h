#include <stddef.h>
#include <sys/types.h>

#include "helpers.h"

// Does this node represent a file or a directory
typedef enum { CFS_FILE, CFS_DIR } node_kind_t;

typedef struct metadata_t {
    // Permissions
    mode_t mode;
    // User id
    uid_t uid;
    // Group id
    gid_t gid;
    // Link number
    nlink_t nlink;
    // Size
    off_t size;
} metadata_t;

// Dynamically sized array containing the children of a node
typedef struct children {
    // The number of children
    size_t len;
    // The maximum number of children that can stored before needing
    // reallocation.
    size_t cap;
    struct cfs_node **items;
} children;

// Create an empty instance of the children struct
void init_children(struct children *);

// Insert a new child to the children array
int insert_child(struct children *, struct cfs_node *);

void free_all_children(struct children *);

// Return the index at which you can find the child with the given name.
//
// If it is not found, then -1 will be returned.
int find_child_with_name(struct children *c, const char *name);

typedef union node_data {
    struct cf_str_t file_content;
    struct children dir_children;
} node_data;

// This contains the information that all nodes (file and dir) have in common.
typedef struct cfs_node {
    metadata_t meta;
    node_kind_t node_k;

    struct cf_str_t name;
    struct cfs_node *parent;
    union node_data data;
} cfs_node;

int is_file(cfs_node *n);
int is_dir(cfs_node *n);

// Initialize a node. After initialization, `parent` will be NULL. Parent shuold
// be set when inserting the node into the list of children.
void init_node(struct cfs_node *, metadata_t, node_kind_t, const char *);
// Create a new node. After creating, `parent` will be NULL. Parent shuold
// be set when inserting the node into the list of children.
cfs_node *new_node(metadata_t, node_kind_t, const char *);

// Given nodes `r` and `c`, insert `c` as a child of `r`. From now on,
// `r.data.dir_children` will be responsible for freeing `c`.
int adopt_child(struct cfs_node *r, struct cfs_node *c);
// Insert a new empty directory with a given name and given metadata as a child
// of `r`
int cfs_node_mkdir(struct cfs_node *r, metadata_t, const char *);
// Insert a new empty file with a given name and given metadata as a child
// of `r`
int cfs_node_touch(struct cfs_node *r, metadata_t, const char *);

// Detach a node from parent. Of couse, this does not work with the root
// node as it does not have a parent.
//
// This will not free the node from memory.
int detach(struct cfs_node *);

// Remove an *empty* directory.
//
// This shuld give an error if r is not empty, or not a dir.
int cfs_node_rmdir(struct cfs_node *r);

void free_node(struct cfs_node *);

typedef struct cfs_tree {
    struct cfs_node *root_node;
} cfs_tree;

cfs_tree *new_tree(void);

// Given a path, return a pointer to the corresponding node. Will return NULL
// if the node does not exist.
cfs_node *find_node_by_path(const cfs_tree *, const char *);

// Given some path, return a pointer to the second last node, as well as the name of the
// last node.
//
// For example "/foo/bar" will return a pointer to "/foo" and will save "bar" to fin_name.
//
// This is useful when creating new nodes, as the full path will not exist, but the parent
// path needs to exist.
cfs_node *find_parent_node_by_path(const cfs_tree *tree, const char *path, const char **fin_name);

cfs_node *mkdir_path(const cfs_tree *, const char *);
