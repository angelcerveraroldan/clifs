#include "clifs_tree.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void init_children(struct children *c) 
{
	c->cap = 0;
	c->len = 0;
	c->items = NULL;
}

int insert_child(struct children *c, struct cfs_node *n)
{
	// If we dont have enough space, we will reallocate
	if (c->len + 1 > c->cap) 
	{
		c->cap = c->cap == 0 ? 4 : c->cap * 2;
		size_t newsize = c->cap * sizeof *c->items;
		void *new_ptr = realloc(c->items, newsize);
		if (new_ptr == NULL) return -ENOMEM;
		c->items = new_ptr;
	}
	c->items[c->len] = n;
	c->len ++;
	return 0;
}

void free_all_children(struct children * c)
{
	if (c==NULL) return;

	for (size_t i = 0; i < c->len; i ++)
	{
		cfs_node *node = c->items[i];
		free_node(node);
	}

	free(c->items);

	c->items= NULL;
	c->cap = 0;
	c->len = 0;
}

// Quite a slow check, but I dont expect it will ever have to handle
// large values of n. This should later be optimized.
int find_child_with_name(struct children *c, const char *name)
{
	for (size_t i = 0; i < c->len; i ++)
		if (strcmp(c->items[i]->name.data, name) == 0) return i;
	return -1;
}

int find_child_with_span(struct children *c, const char *name, size_t len)
{

	for (size_t i = 0; i < c->len; i ++)
	{
		if (c->items[i]->name.len != len) continue;
		if (memcmp(c->items[i]->name.data, name, len) == 0) return i;
	}
	return -1;

}

int is_file(cfs_node *n) { return n->node_k == CFS_FILE; }
int is_dir (cfs_node *n) { return n->node_k == CFS_DIR ; }

void init_node(struct cfs_node *node, metadata_t meta, node_kind_t node_k, const char * name)
{
	node->meta = meta;
	node->node_k = node_k;
	cstr_set(&node->name, name);
	node->parent = NULL;

	if (node_k == CFS_DIR) init_children(&node->data.dir_children);
	else cstr_set(&node->data.file_content, "");
}

cfs_node *new_node(metadata_t meta, node_kind_t node_k, const char * name)
{
	cfs_node *node = malloc(sizeof(cfs_node));
	init_node(node, meta, node_k, name);
	return node;
}

int rename(struct cfs_node *n, const char *new_name)
{
	// Cannot rename root node
	if (n == NULL || n->parent == NULL) return -EINVAL;

	if (strlen(new_name) > 255) return -ENAMETOOLONG;
	if (strcmp(new_name, "") == 0 || strcmp(new_name, ".") == 0 || strcmp(new_name, "..") == 0) 
		return -EINVAL;

	// Check that the name does not already exist as a sibling
	int index = find_child_with_name(&n->parent->data.dir_children, new_name);
	if (index != -1) return -EEXIST;

	// Edit the name of the node
	return cstr_set(&n->name, new_name);
}

int adopt_child(struct cfs_node *r, struct cfs_node *c)
{
	// A file cannot have a child
	if (is_file(r) || r == NULL || c == NULL) return -EINVAL;

	// Name already in use
	if (find_child_with_name(&r->data.dir_children, c->name.data) != -1)
		return -EEXIST;

	int e = insert_child(&r->data.dir_children, c);
	if (e != 0) return e;
	c->parent = r;
	return 0;
}

int cfs_node_mkdir(struct cfs_node *r, metadata_t meta, const char *name)
{
	cfs_node *nn = new_node(meta, CFS_DIR, name);
	return adopt_child(r, nn);
}

int cfs_node_touch(struct cfs_node *r, metadata_t meta, const char *name)
{
	cfs_node *nn = new_node(meta, CFS_FILE, name);
	return adopt_child(r, nn);
}

// Detach a node from parent. Of couse, this does not work with the root
// node as it does not have a parent.
int detach(struct cfs_node *r)
{
	if (r == NULL || r->parent == NULL) return -EINVAL;

	children *c = &r->parent->data.dir_children;
	int index = find_child_with_name(c, r->name.data);

	// This shuold never happen, the name must be found.
	if (index == -1) return -ENOENT;

	// This is safe from underflow, as parent must have at least r as a child
	size_t last = c->len - 1;

	if ((size_t) index != last) 
	{
		c->items[index] = c->items[last];
		c->items[last]  = NULL;
	}

	if (is_dir(r)) r->parent->meta.nlink --;
	r->parent = NULL;
	c->len --;

	return 0;
}

int cfs_node_rmdir(struct cfs_node *r)
{
	if (r == NULL || is_file(r)) return -EINVAL;
	if (r->data.dir_children.len != 0) return -ENOTEMPTY;

	int d = detach(r);
	if (d != 0) return d;

	free_node(r);
	return 0;
}

void free_node(struct cfs_node *node)
{
	// Start by freeing the data
	if (node->node_k == CFS_DIR) free_all_children(&node->data.dir_children);
	else free_cstr(&node ->data.file_content);

	free_cstr(&node->name);
	free(node);
}

cfs_tree *new_tree(void)
{
	metadata_t meta = {0};
	meta.gid = getgid();
	meta.uid = getuid();
	meta.nlink = 2;
	meta.mode = 0755;
	meta.size = 0;

	cfs_tree *tree = calloc(1, sizeof *tree);
	if (!tree) return NULL;

	tree->root_node = new_node(meta, CFS_DIR, "");
	return tree;
}

cfs_node *find_node_by_path(const cfs_tree *tree, const char *path)
{
	if (!tree || !tree->root_node || !path) return NULL;
	cfs_node *curr_node = tree->root_node;

	// Handle "", and "/"
	if (path[0] == '\0' || (path[0]=='/' && path[1]=='\0')) return curr_node;

	const char *p = path;
	if (*p == '/') p++;

	while (*p)
	{
		const char *start = p;
		while (*p && *p != '/') p++;
		size_t l = (size_t) (p - start);

		// We want to just keep going when we find doubled '/', for example,
		// "foo//bar" should be the same as "foo/bar"
		if (l == 0) { if (*p=='/') p++; continue; }

		// If we find "./", then we just keep going!
		if (l == 1 && start[0] == '.') 
		{
			if (*p == '/') p++;
			continue;
		}

		// Check for ".."
		if (l == 2 && start[0] == '.' && start[1] == '.')
		{
			if (!curr_node->parent) return NULL;
			curr_node = curr_node->parent;
			if (*p == '/') p++;
			continue;
		}

		// We want to go to a child of the current node
		if (is_file(curr_node)) return NULL;
		int index = find_child_with_span(&curr_node->data.dir_children, start, l);
		// Child not found
		if (index == -1) return NULL;
		curr_node = curr_node->data.dir_children.items[index];
		p ++;
	}

	return curr_node;
}

cfs_node *find_parent_node_by_path(const cfs_tree *tree, const char *path, const char **fin_name)
{
	int sep = last_dash(path);
	if (sep == -1)
	{
		*fin_name = path;
		return tree->root_node;
	}

	char *parent_path = malloc((sep+1) * sizeof(char));
	if (!parent_path) return NULL;

	memcpy(parent_path, path, sep);
	parent_path[sep] = '\0';
	*fin_name = path + sep + 1;
	cfs_node *node = find_node_by_path(tree, parent_path);
	free(parent_path);
	return node;
}

