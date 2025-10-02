#include "clifs_tree.h"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

CFS_NODE::CFS_NODE(CFS_NODE *parent, Metadata md, NODE_KIND nk,
                   std::string name)
    : kind(nk), name(name), parent(parent) {
  this->meta = md;
}

Metadata make_metadata(mode_t mode, uid_t uid, gid_t gid, nlink_t nlink,
                       off_t size) {
  Metadata meta;
  meta.mode = mode;
  meta.uid = uid;
  meta.gid = gid;
  meta.nlink = nlink;
  meta.size = size;
  return meta;
}

std::unique_ptr<CFS_NODE> CFS_NODE::make_dir(CFS_NODE *parent, std::string name,
                                             mode_t mode, uid_t uid,
                                             gid_t gid) {
  Metadata meta = make_metadata(mode, uid, gid, 2, 0);
  return std::make_unique<CFS_NODE>(parent, meta, NODE_KIND::DIR,
                                    std::move(name));
}

std::unique_ptr<CFS_NODE> CFS_NODE::make_file(CFS_NODE *parent,
                                              std::string name, mode_t mode,
                                              uid_t uid, gid_t gid,
                                              off_t size) {
  Metadata meta = make_metadata(mode, uid, gid, 1, size);
  return std::make_unique<CFS_NODE>(parent, meta, NODE_KIND::FILE,
                                    std::move(name));
}

CFS_NODE *CFS_NODE::adopt_child(std::unique_ptr<CFS_NODE> child) {
  if (child == nullptr)
    return nullptr;

  bool is_dir = child->is_dir();

  // The name is already in use
  if ((children.find(child->name) != children.end()) || is_file()) {
    return nullptr;
  }

  // Make sure that we have the right parent at all times
  child->parent = this;

  // Add the child to the list of children
  auto [it, inserted] = children.emplace(child->name, std::move(child));

  // Error during insertion
  if (!inserted)
    return nullptr;

  if (is_dir)
    nlink_add(1);

  return it->second.get();
}

CFS_NODE *CFS_NODE::mkdir(std::string name, mode_t mode, uid_t uid, gid_t gid) {
  return adopt_child(make_dir(this, name, mode, uid, gid));
}

int CFS_NODE::rmdir() {
  if (is_file() || !children.empty())
    return -EINVAL;

  detach_from_parent();
  return 0;
}

CFS_NODE *CFS_NODE::touch(std::string name, mode_t mode, uid_t uid, gid_t gid,
                          off_t size) {
  return adopt_child(make_file(this, name, mode, uid, gid, size));
}

void CFS_NODE::to_stat(struct stat &st) const {
  std::memset(&st, 0, sizeof(st));
  st.st_uid = meta.uid;
  st.st_gid = meta.gid;
  st.st_nlink = meta.nlink;
  st.st_size = meta.size;
  st.st_mode = ((kind == NODE_KIND::DIR) ? S_IFDIR : S_IFREG) | meta.mode;
}

CFS_NODE *CFS_NODE::find_child(std::string name) {
  auto x = children.find(name);
  if (x == children.end()) {
    return nullptr;
  }
  return x->second.get();
}

std::vector<std::string> CFS_NODE::children_names() {
  std::vector<std::string> names;
  names.reserve(children.size());
  for (auto &kv : children)
    names.emplace_back(kv.first);
  return names;
}

int CFS_NODE::rename(name_t new_name) {
  // You cannot rename the root node
  if (parent == nullptr)
    return -EINVAL;

  // Nothing to do
  if (name == new_name)
    return 0;

  // Path already exists
  if (parent->find_child(new_name) != nullptr)
    return -EEXIST;

  // Make sure to change the parent children name
  //
  // We know that the parents children map will contain the key `name`
  auto it = parent->children.find(name);
  std::unique_ptr<CFS_NODE> unique_this = std::move(it->second);
  parent->children.erase(it);
  this->name = new_name;
  auto [_it, inserted] =
      parent->children.emplace(new_name, std::move(unique_this));

  // There was an error when inserting the new_name -- This is a fatal error
  // with loss of data ...
  //
  // Later there should be some sort of rollback if this happens
  if (!inserted) {
    return -EIO;
  }

  return 0;
}

std::unique_ptr<CFS_NODE> CFS_NODE::detach_from_parent() {
  if (!parent)
    return nullptr;

  auto uptr = std::move(parent->children.at(name));
  // Remove the key
  parent->children.erase(name);
  return uptr;
}

std::vector<std::string> path_components(std::string path) {
  std::vector<std::string> components;
  std::string next;

  for (char c : path) {
    if (c == '/') {
      if (!next.empty())
        components.emplace_back(next);

      next.clear();
    } else
      next.push_back(c);
  }

  if (!next.empty())
    components.emplace_back(next);

  return components;
}

std::string parent_path(std::string path) {
  auto components = path_components(path);
  components.pop_back();
  std::string parent = "/";
  for (auto &comp : components) {
    parent.append(comp);
    parent.push_back('/');
  }
  return parent;
}

CFS_TREE::CFS_TREE()
    : root_node(nullptr, make_metadata(0555, getuid(), getgid(), 2, 0),
                NODE_KIND::DIR, "/") {}

CFS_NODE *CFS_TREE::find(std::string path) {
  CFS_NODE *node = &root_node;
  std::vector<std::string> components = path_components(path);
  for (std::string child : components) {
    CFS_NODE *cnode = node->find_child(child);

    // Path not found!
    if (cnode == nullptr)
      return nullptr;
    node = cnode;
  }

  return node;
}

CFS_NODE *CFS_TREE::touch_p(std::string path) {
  auto components = path_components(path);
  std::string parent_path = parent_path(path);
  std::string file_name = components.back();
  CFS_NODE *parent = mkdir_p(parent_path);

  if (parent == nullptr || parent->is_file())
    return nullptr;

  auto uid = getuid(), gid = getgid();
  CFS_NODE *new_file = parent->touch(file_name, 0555, uid, gid);
  return new_file;
}

CFS_NODE *CFS_TREE::mkdir_p(std::string path) {
  CFS_NODE *node = &root_node;
  auto components = path_components(path);
  for (auto comp : components) {
    CFS_NODE *cnode = node->find_child(comp);

    if (cnode != nullptr && cnode->is_dir())
      continue;
    else if (cnode != nullptr && cnode->is_file())
      return nullptr;

    auto uid = getuid(), gid = getgid();
    cnode = node->mkdir(comp, 0555, uid, gid);
  }
  return node;
}

int CFS_TREE::rmdir_p(path_t path) {
  CFS_NODE *node = find(path);

  // Path not found!
  if (node == nullptr)
    return -EINVAL;

  return node->rmdir();
}

int CFS_TREE::rename_p(path_t from, path_t to) {
  // From path does not exists
  CFS_NODE *from_leaf = find(from);
  if (!from_leaf)
    return -EINVAL;

  auto p_from = parent_path(from), p_to = parent_path(to);
  auto pfrom_node = find(p_from), pto_node = find(p_to);

  // The from nodes needs to be directories
  if (pfrom_node == nullptr || pto_node == nullptr || pfrom_node->is_file() ||
      pto_node->is_file())
    return -EINVAL;

  if (from_leaf->is_dir())
    pfrom_node->nlink_add(-1);

  // This should later have some error handling
  pto_node->adopt_child(from_leaf->detach_from_parent());
  return 0;
}
