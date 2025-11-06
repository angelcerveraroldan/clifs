#include "helpers.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>


int cstr_set(cfstr_t *s, const char *new_str)
{
	if (s == NULL || new_str == NULL) return -EINVAL;

	size_t size = strlen(new_str);
	if (s->cap <= size+1)
	{
		size_t ncap = size+1;
		void *new_ptr = realloc(s->data, ncap * sizeof(char));
		if (new_ptr == NULL) return -ENOMEM;

		s->cap = ncap;
		s->data = new_ptr;
	}
	memcpy(s->data, new_str, size+1);
	s->len = size;
	return 0;
}

void free_cstr(cfstr_t *s)
{
	if (s==NULL) return;
	free(s->data);
	s->data=NULL;
	s->cap=0;
	s->len=0;
}

// Returns index of the slash before the final path component (leaf).
// Trims trailing '/' (except root). Returns -1 if no parent/leaf split exists.
int last_dash(const char *path)
{
	if (!path) return -1;
	ssize_t n = strlen(path);
	if (n == 0) return -1;

	// Trim trailing slashes but keep a single "/" as-is
	while (n > 1 && path[n - 1] == '/') n--;

	// If now just "/", or no slash at all → no split
	if (n == 1 && path[0] == '/') return -1;

	// Find the slash immediately before the leaf
	for (ssize_t i = (ssize_t)n - 1; i >= 0; --i) {
		if (path[i] == '/') {
			// i is the separator; leaf starts at i+1 (guaranteed non-'/' due to trim)
			return (int)i;
		}
	}
	return -1;
}
