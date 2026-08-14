#include "clifs_tree.h"
#include "file_descriptor.h"

#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 31
#endif

#include <asm-generic/errno-base.h>
#include <assert.h>
#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Just for the time being ...
#ifndef S_IFDIR
#define S_IFDIR 0040000
#endif
#ifndef S_IFREG
#define S_IFREG 0100000
#endif

typedef struct cfs_state {
    struct cfs_tree *tree;
    struct fd_table table;
} cfs_state;

static cfs_tree *ctx_tree(void) {
    cfs_state *pd = fuse_get_context()->private_data;
    return pd->tree;
}

/*
static fd_table *ctx_table(void)
{

        cfs_state *pd = fuse_get_context()->private_data;
        return &pd->table;
}
*/

void set_stat(const cfs_node *node, struct stat *st) {
    memset(st, 0, sizeof(*st));
    metadata_t meta = node->meta;
    st->st_uid = meta.uid;
    st->st_gid = meta.gid;
    st->st_nlink = meta.nlink;
    st->st_size = meta.size;
    st->st_mode = ((node->node_k == CFS_DIR) ? S_IFDIR : S_IFREG) | meta.mode;
}

static int cfs_get_attr(const char *path, struct stat *st, struct fuse_file_info *fi) {
    (void)fi;

    cfs_tree *t = ctx_tree();
    cfs_node *node = find_node_by_path(t, path);
    if (!node) return -ENOENT;
    set_stat(node, st);
    return 0;
}

static int cfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t off,
                       struct fuse_file_info *fi, enum fuse_readdir_flags frf) {
    (void)fi;
    (void)off;
    (void)frf;

    cfs_tree *t = ctx_tree();
    cfs_node *node = find_node_by_path(t, path);
    if (!node) return -ENOENT;

    filler(buffer, ".", NULL, 0, 0);
    filler(buffer, "..", NULL, 0, 0);

    if (is_file(node)) return -ENOTDIR;
    children cs = node->data.dir_children;
    for (size_t i = 0; i < cs.len; i++) {
        cfs_node *c = cs.items[i];
        filler(buffer, c->name.data, NULL, 0, 0);
    }

    return 0;
}

static int cfs_mkdir(const char *path, mode_t mode) {
    cfs_tree *t = ctx_tree();
    const char *buffer = NULL;
    cfs_node *n = find_parent_node_by_path(t, path, &buffer);

    if (!buffer) return -EINVAL;
    if (!n) return -ENOENT;
    if (is_file(n)) return -ENOTDIR;

    metadata_t meta = {
        .gid = fuse_get_context()->gid,
        .uid = fuse_get_context()->uid,
        .mode = mode,
        .nlink = 2,
        .size = 0,
    };

    return cfs_node_mkdir(n, meta, buffer);
}

static int cfs_rmdir(const char *path) {
    cfs_tree *t = ctx_tree();
    cfs_node *node = find_node_by_path(t, path);
    if (!node) return -ENOENT;
    return cfs_node_rmdir(node);
}

static struct fuse_operations cfs_ops = {
    .getattr = cfs_get_attr,
    .readdir = cfs_readdir,
    .mkdir = cfs_mkdir,
    .rmdir = cfs_rmdir,
};

int main(int argc, char *argv[]) {
    cfs_state state = {.table = new_empty(), .tree = new_tree()};
    fuse_main(argc, argv, &cfs_ops, &state);
    return 0;
}
