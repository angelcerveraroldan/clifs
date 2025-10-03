#include "clifs_tree.h"
#include "file_desc.h"
#include "params.h"
#include "state.h"
#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include <fuse3/fuse.h>

static CFS_TREE *ctx_tree() {
  return &static_cast<CFS_STATE *>(fuse_get_context()->private_data)->tree;
}

static FD_TABLE *ctx_filedesc_table() {
  return &static_cast<CFS_STATE *>(fuse_get_context()->private_data)->file_desc;
}

// Get file attributes
//
// Similar to the stat function.
static int cf_getatt(const char *path, struct stat *st,
                     [[maybe_unused]] struct fuse_file_info *fi) {
  std::memset(st, 0, sizeof(*st));
  if (auto *node = ctx_tree()->find(path)) {
    node->to_stat(*st);
    return 0;
  }
  return -ENOENT;
}

static int cf_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
                      off_t, struct fuse_file_info *, enum fuse_readdir_flags) {
  auto *node = ctx_tree()->find(path);

  // Path does not exist
  if (node == nullptr) {
    return -ENOENT;
  }

  fuse_fill_dir_flags EMPTY = (fuse_fill_dir_flags)0;

  // self and parent
  filler(buffer, ".", NULL, 0, EMPTY);
  filler(buffer, "..", NULL, 0, EMPTY);

  // Children
  for (const auto &name : node->children_names()) {
    filler(buffer, name.c_str(), NULL, 0, EMPTY);
  }

  return 0;
}

static int cf_mkdir(const char *path, mode_t mode) {
  auto components = path_components(path);
  if (components.empty())
    return -EINVAL;

  std::string pp = parent_path(path);
  std::string new_dir_name = components.back();

  CFS_TREE *tree = ctx_tree();
  CFS_NODE *parent = tree->find(pp);

  // Parent does not exist
  if (!parent)
    return -ENOENT;
  // Cannot build inside a file
  if (parent->is_file())
    return -ENOTDIR;
  // Directory already exists
  if (parent->find_child(new_dir_name))
    return -EEXIST;

  CFS_NODE *new_node = parent->mkdir(
      new_dir_name, mode, fuse_get_context()->uid, fuse_get_context()->gid);
  // Error making the new dir
  if (!new_node)
    return -EIO;

  // All went well!
  return 0;
}

static int cf_rmdir(const char *path) { return ctx_tree()->rmdir_p(path); }

static int cf_rename(const char *p1, const char *p2, unsigned int /*flags*/) {
  return ctx_tree()->rename_p(p1, p2);
}

static int cf_open(const char *path, struct fuse_file_info *ffi) {
  CFS_NODE *node = ctx_tree()->find(path);
  FD_TABLE *table = ctx_filedesc_table();

  if (!node)
    return -ENOENT;

  if (node->is_dir())
    return -EISDIR;

  ffi->fh = table->open(node, ffi->flags);
  return 0;
}

static int cf_read([[maybe_unused]] const char *path, char *buffer, size_t size,
                   off_t offset, struct fuse_file_info *ffi) {
  auto *table = ctx_filedesc_table();
  auto *file_handle = table->get(ffi->fh);

  if (!file_handle)
    return -EBADF;

  CFS_NODE *node = file_handle->node;
  std::string val = node->get_data().value_or("");
  size_t len = val.length();

  if ((size_t)offset >= len)
    return 0;

  if (offset + size > len)
    size = len - offset;

  memcpy(buffer, node->get_data().value_or("").c_str() + offset, size);
  return size;
}

static fuse_operations clifs_fuse_operations() {
  fuse_operations ops{};

  ops.getattr = cf_getatt;
  ops.readdir = cf_readdir;
  ops.mkdir = cf_mkdir;
  ops.rename = cf_rename;
  ops.rmdir = cf_rmdir;
  ops.open = cf_open;
  ops.read = cf_read;

  return ops;
}

// TODO: This is just a placeholder for now.
void help_message() {
  std::cout << "Please make sure you are entering the args correctly"
            << std::endl;
}

int main(int argc, char *argv[]) {
  CFS_STATE data;
  fuse_operations ops = clifs_fuse_operations();
  fuse_main(argc, argv, &ops, &data);
  return 0;
}
