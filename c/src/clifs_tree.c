#include "clifs_tree.h"
#include <asm-generic/errno-base.h>
#include <stdlib.h>
#include <string.h>

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

int adopt_child(struct cfs_node *r, struct cfs_node *c)
{
	// A file cannot have a child
	if (is_file(r) || r == NULL || c == NULL) return -EINVAL;

	// Name already in use
	if (find_child_with_name(&r->data.dir_children, c->name.data) != 0)
		return -EINVAL;

	int e = insert_child(&r->data.dir_children, c);
	if (e != 0) return e;
	c->parent = r;
	return 0;
}

int mkdir(struct cfs_node *r, metadata_t meta, const char *name)
{
	cfs_node *nn = new_node(meta, CFS_DIR, name);
	return adopt_child(r, nn);
}

int touch(struct cfs_node *r, metadata_t meta, const char *name)
{
	cfs_node *nn = new_node(meta, CFS_FILE, name);
	return adopt_child(r, nn);
}

void free_node(struct cfs_node *node)
{
	// Start by freeing the data
	if (node->node_k == CFS_DIR) free_all_children(&node->data.dir_children);
	else free_cstr(&node ->data.file_content);

	free_cstr(&node->name);
	free(node);
}
