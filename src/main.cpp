#include "clifs_data.h"
#include "params.h"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include <fuse3/fuse.h>

CFS_DATA *context_data() {
  void *private_data = fuse_get_context()->private_data;
  return static_cast<CFS_DATA *>(private_data);
}

// Get file attributes
//
// Similar to the stat function.
static int cf_getatt(const char *path, struct stat *st,
                     [[maybe_unused]] struct fuse_file_info *fi) {
  std::memset(st, 0, sizeof(*st));

  if (strcmp(path, "/") == 0) {
    st->st_mode =
        S_IFDIR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    st->st_nlink = 2;

    // User id and group id
    st->st_uid = getuid();
    st->st_gid = getgid();

    return 0;
  }

  if (strcmp(path, "/hello") == 0) {
    static char message[] = "hello there!";
    st->st_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH;
    st->st_nlink = 1;
    // User id and group id
    st->st_uid = getuid();
    st->st_gid = getgid();
    st->st_size = sizeof(message) - 1;
    return 0;
  }

  return -ENOENT;
}

static int cf_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
                      off_t, struct fuse_file_info *, enum fuse_readdir_flags) {
  if (strcmp(path, "/") != 0)
    return -ENOENT;

  fuse_fill_dir_flags EMPTY = (fuse_fill_dir_flags)0;

  // self and parent
  filler(buffer, ".", NULL, 0, EMPTY);
  filler(buffer, "..", NULL, 0, EMPTY);

  // Test file for now
  filler(buffer, "hello", NULL, 0, EMPTY);

  return 0;
}

static fuse_operations clifs_fuse_operations() {
  fuse_operations ops{};

  ops.getattr = cf_getatt;
  ops.readdir = cf_readdir;

  return ops;
}

// TODO: This is just a placeholder for now.
void help_message() {
  std::cout << "Please make sure you are entering the args correctly"
            << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    help_message();
    return 1;
  }

  auto mnt_pt = argv[1];
  CFS_DATA data = CFS_DATA(mnt_pt);
  fuse_operations ops = clifs_fuse_operations();

  fuse_main(argc, argv, &ops, &data);

  return 0;
}
