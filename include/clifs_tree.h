#ifndef _TREE_CLIFS_H
#define _TREE_CLIFS_H

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

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
  CFS_NODE(Metadata, NODE_KIND, std::string);

  static std::unique_ptr<CFS_NODE> make_dir(std::string, mode_t, uid_t, gid_t);
  static std::unique_ptr<CFS_NODE> make_file(std::string, mode_t, uid_t, gid_t,
                                             off_t size = 0);

  bool is_file() const noexcept { return kind == NODE_KIND::FILE; }
  bool is_dir() const noexcept { return kind == NODE_KIND::DIR; }

  Metadata &metadata() noexcept { return meta; }

  // Add a new file or directory (add a generic child)
  CFS_NODE *add_child(std::string, std::unique_ptr<CFS_NODE>);

  // Make a new subdirectory
  CFS_NODE *mkdir(std::string, mode_t mode, uid_t, gid_t);

  // Make a new file
  CFS_NODE *touch(std::string name, mode_t mode, uid_t, gid_t, off_t size = 0);

  CFS_NODE *find_child(std::string name);

  void to_stat(struct stat &st) const;

  std::vector<std::string> children_names();

private:
  NODE_KIND kind;
  Metadata meta;
  // Name of the directory or file
  std::string name;

  std::unordered_map<std::string, std::unique_ptr<CFS_NODE>> children;
  // Files may contain some data in the form of bytes, directories will contain
  // no data.
  std::optional<std::vector<std::byte>> data;
};

class CFS_TREE {
public:
  CFS_TREE();
  CFS_TREE(CFS_NODE root_node);
  CFS_NODE *find(std::string path);

  CFS_NODE *touch_p(std::string path);
  CFS_NODE *mkdir_p(std::string path);

private:
  CFS_NODE root_node;
};

// Some helper functions
std::string parent(std::string);
std::vector<std::string> split_name(std::string);

#endif
