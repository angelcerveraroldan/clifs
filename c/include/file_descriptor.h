#ifndef _CLIFS_FILE_DESCRIPTOR_H
#define _CLIFS_FILE_DESCRIPTOR_H

#include <stdint.h>
#include <stdlib.h>

typedef struct file_handle {
    struct cfs_node *node;
    int flags;
    int in_use;
} file_handle;

typedef struct fd_table {
    size_t handles_cap;
    file_handle *handles;
} fd_table;

fd_table new_empty(void);

// We have opened a file, create a new File Handle, and add it to the
// list.
uint64_t open_node_file(struct fd_table *, struct cfs_node *, int);

// Get file handle by index, if out of bounds, NULL will be returned.
file_handle *get_by_id(struct fd_table *, uint64_t id);

// A file has been closed
void close_by_id(struct fd_table *, uint64_t id);

#endif
