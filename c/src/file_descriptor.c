#include "file_descriptor.h"

fd_table new_empty(void)
{
	fd_table ft = { .handles_cap = 0, .handles = NULL };
	return ft;
}

uint64_t open_node_file(struct fd_table * ft, struct cfs_node *n, int flags)
{
	file_handle fh = { .node = n, .flags = flags, .in_use = 1 };


	// Check if there is any free file_handles that we can re-use
	for (size_t i = 0; i < ft->handles_cap; i ++)
		if (!ft->handles[i].in_use) 
		{ 
			ft->handles[i] = fh;
			return (uint64_t) i;
		}

	size_t old_cap = ft->handles_cap;
	size_t ncap = old_cap ? old_cap * 2 : 4;
	file_handle *np = realloc(ft->handles, ncap * sizeof *np);
	if (!np) return UINT64_MAX;

	for (size_t i = old_cap; i < ncap; i++) np[i] = (file_handle){0};
	ft->handles = np;
	ft->handles_cap = ncap;

	size_t id = old_cap;
	ft->handles[id] = fh;
	return (uint64_t)id;
}

file_handle *get_by_id(struct fd_table *ft, uint64_t id)
{
	if (id >= ft->handles_cap) return NULL;
	file_handle *fh = &ft->handles[id];
	if (!fh->in_use) return NULL;
	else return fh;
}

void close_by_id(struct fd_table *ft, uint64_t id)
{
	if (id >= ft->handles_cap) return;
	ft->handles[id].flags  = 0;
	ft->handles[id].in_use = 0;
	ft->handles[id].node   = NULL;
}
