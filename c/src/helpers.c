#include "helpers.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <stdlib.h>
#include <string.h>

int cstr_set(cstr_t *s, const char *new_str)
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

void free_cstr(cstr_t *s)
{
	if (s==NULL) return;
	free(s->data);
	s->data=NULL;
	s->cap=0;
	s->len=0;
}
