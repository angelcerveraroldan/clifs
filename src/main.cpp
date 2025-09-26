#include "params.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/stat.h>

#include <fuse3/fuse.h>

// Get file attributes
//
// Similar to the stat function.
int cf_getatt(const char *path, struct stat *st,
              [[maybe_unused]] struct fuse_file_info *fi) {
  // For now the only valid path is "/"
  if (strcmp(path, "/")) {
    // 0555 -> read, write, execute
    st->st_mode = S_IFDIR | 0555;
    st->st_nlink = 2;
    return 0;
  }
  return -ENOENT;
}

int main() {
  // Hello!
  std::cout << "Hello!" << std::endl;
  return 0;
}
