#ifndef _TREE_CLIFS_H
#define _TREE_CLIFS_H

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

// A path is just a string
typedef std::string path_t;
// The name of a file or directory
typedef std::string name_t;

enum class NODE_KIND { FILE, DIR };

struct Metadata {
  // Permissions
  mode_t mode;
  // User id
  uid_t uid;
  // Group id
  gid_t gid;
  // Link number
  nlink_t nlink;
  // Size
  off_t size;
};

class CFS_NODE {
public:
  CFS_NODE(CFS_NODE *, Metadata, NODE_KIND, std::string);

  // We cannot copy because of the unique pointer
  CFS_NODE(const CFS_NODE &) = delete;
  CFS_NODE &operator=(const CFS_NODE &) = delete;

  // Moving is fine, but we must move all of the unique_ptrs
  CFS_NODE(CFS_NODE &&) noexcept = default;
  CFS_NODE &operator=(CFS_NODE &&) noexcept = default;

  static std::unique_ptr<CFS_NODE> make_dir(CFS_NODE *, name_t, mode_t, uid_t,
                                            gid_t);
  static std::unique_ptr<CFS_NODE> make_file(CFS_NODE *, name_t, mode_t, uid_t,
                                             gid_t, off_t size = 0);

  bool is_file() const noexcept { return kind == NODE_KIND::FILE; }
  bool is_dir() const noexcept { return kind == NODE_KIND::DIR; }

  Metadata &metadata() noexcept { return meta; }

  // Add a new file or directory (add a generic child)
  //
  // If the child adopted is a dir, this will increase nlink
  CFS_NODE *adopt_child(std::unique_ptr<CFS_NODE>);

  // Make a new subdirectory
  CFS_NODE *mkdir(name_t, mode_t mode, uid_t, gid_t);

  // Remove an *empty* directory
  int rmdir();

  // Make a new file
  CFS_NODE *touch(name_t name, mode_t mode, uid_t, gid_t, off_t size = 0);

  CFS_NODE *find_child(name_t name);

  void to_stat(struct stat &st) const;

  std::vector<name_t> children_names();

  int rename(name_t);

  // move the unique pointer to this node.
  std::unique_ptr<CFS_NODE> detach_from_parent();

  /*
   * Modifies for Metadata
   * */

  // Add to the nlink
  void nlink_add(int by) { meta.nlink += by; }

  std::optional<std::string> get_data() const { return data; }

private:
  NODE_KIND kind;
  Metadata meta;
  // Name of the directory or file
  std::string name;

  // We are guaranteed that the parent pointer is valid as long a this is valid
  // (parents outlives child)
  //
  // null if this is the root
  CFS_NODE *parent;
  std::unordered_map<std::string, std::unique_ptr<CFS_NODE>> children;
  // Files may contain some data in the form of bytes, directories will contain
  // no data.
  std::optional<std::string> data;
};

class CFS_TREE {
public:
  CFS_TREE();
  CFS_TREE(CFS_NODE root_node);

  // Find a node with a given absolute path
  CFS_NODE *find(path_t);

  // Given an absolute path to a non-existant file, create said file
  //
  // Will return void if:
  // - File already exists
  // - Parent could not be created
  CFS_NODE *touch_p(path_t);
  // Given an absolute path to a directory, create said dierectory
  //
  // Will return void if:
  // - Directory already exists
  // - Parent could not be created
  CFS_NODE *mkdir_p(path_t);

  // Remove an *empty* directory
  int rmdir_p(path_t);

  // Given two paths, rename all directories in the from path such that it
  // matches the to path
  int rename_p(path_t from, path_t to);

private:
  CFS_NODE root_node;
};

/*
 *  HELPER FUNCTIONS
 * */

// Give some absolute path, get the parent path
std::string parent_path(path_t);

// Given some path, divide it into its segments
//
// I.e. /this/that/hello.txt -> {this, that, hello.txt}
std::vector<std::string> path_components(path_t);

#endif
