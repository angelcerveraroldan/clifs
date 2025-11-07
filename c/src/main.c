#include <asm-generic/errno-base.h>
#include "clifs_tree.h"
#include "file_descriptor.h"
#include "params.h"
#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

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

static cfs_tree *ctx_tree(void)
{
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

void set_stat(const cfs_node *node, struct stat *st)
{
	memset(st, 0, sizeof(*st));
	metadata_t meta = node->meta;
	st->st_uid = meta.uid;
	st->st_gid = meta.gid;
	st->st_nlink = meta.nlink;
	st->st_size = meta.size;
	st->st_mode = ((node->node_k == CFS_DIR) ? S_IFDIR: S_IFREG) | meta.mode;
}

static int cfs_get_attr(const char *path, struct stat *st, struct fuse_file_info *fi)
{
	(void) fi;

	cfs_tree *t = ctx_tree();
	cfs_node *node = find_node_by_path(t, path);
	if (!node) return -ENOENT;
	set_stat(node, st);
	return 0;
}

static struct fuse_operations cfs_ops = {
	.getattr = cfs_get_attr,
};

int main(int argc, char *argv[])
{
	cfs_state state = { .table = new_empty(), .tree = new_tree() };
	printf("Ok, lets go:\n");
	printf("Table hc: %zu", state.table.handles_cap);
	fflush(stdout);
	printf("tree: %s",  state.tree->root_node->name.data);
	fflush(stdout);

	fuse_main(argc, argv, &cfs_ops, &state);
	return 0;
}

