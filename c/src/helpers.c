#include "helpers.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

int cstr_set(cfstr_t *s, const char *new_str) {
    if (!s || !new_str) return -EINVAL;

    size_t n = strlen(new_str);
    size_t need = n + 1;

    if (s->cap < need) {
        char *np = (char *)realloc(s->data, need);
        if (!np) return -ENOMEM;
        s->data = np;
        s->cap = need;
    }

    // copy including NUL terminator
    memcpy(s->data, new_str, need);
    s->len = n;
    return 0;
}

void free_cstr(cfstr_t *s) {
    if (!s) return;
    free(s->data);
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
}

int last_dash(const char *path) {
    if (!path) return -1;

    size_t n = strlen(path);
    if (n == 0) return -1;

    // Trim trailing slashes but keep single "/" as root
    while (n > 1 && path[n - 1] == '/') n--;

    if (n == 1 && path[0] == '/') return -1;

    // Find slash immediately before the leaf (scan backwards)
    for (size_t i = n; i-- > 0;) {
        if (path[i] == '/') return (int)i;
    }
    return -1;
}
