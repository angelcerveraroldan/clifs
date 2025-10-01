#include "clifs_tree.h"
#include <cstddef>
#include <cstring>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

CFS_NODE::CFS_NODE(Metadata md, NODE_KIND nk, std::string name)
    : kind(nk), name(name) {
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

std::unique_ptr<CFS_NODE> CFS_NODE::make_dir(std::string name, mode_t mode,
                                             uid_t uid, gid_t gid) {
  Metadata meta = make_metadata(mode, uid, gid, 2, 0);
  return std::make_unique<CFS_NODE>(meta, NODE_KIND::DIR, std::move(name));
}

std::unique_ptr<CFS_NODE> CFS_NODE::make_file(std::string name, mode_t mode,
                                              uid_t uid, gid_t gid,
                                              off_t size) {
  Metadata meta = make_metadata(mode, uid, gid, 1, size);
  return std::make_unique<CFS_NODE>(meta, NODE_KIND::FILE, std::move(name));
}

CFS_NODE *CFS_NODE::add_child(std::string name,
                              std::unique_ptr<CFS_NODE> child) {
  bool is_dir = child->is_dir();
  // The name is already in use
  if ((children.find(name) != children.end()) || is_file()) {
    return nullptr;
  }

  // Add the child to the list of children
  auto [it, inserted] = children.emplace(std::move(name), std::move(child));

  // Error during insertion
  if (!inserted)
    return nullptr;

  if (is_dir)
    this->meta.nlink++;

  return it->second.get();
}

CFS_NODE *CFS_NODE::mkdir(std::string name, mode_t mode, uid_t uid, gid_t gid) {
  return add_child(name, make_dir(name, mode, uid, gid));
}

CFS_NODE *CFS_NODE::touch(std::string name, mode_t mode, uid_t uid, gid_t gid,
                          off_t size) {
  return add_child(name, make_file(name, mode, uid, gid, size));
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

std::vector<std::string> split_name(std::string path) {
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

std::string parent(std::string path) {
  auto components = split_name(path);
  components.pop_back();
  std::string parent = "/";
  for (auto &comp : components) {
    parent.append(comp);
    parent.push_back('/');
  }
  return parent;
}

CFS_TREE::CFS_TREE()
    : root_node(make_metadata(0555, getuid(), getgid(), 2, 0), NODE_KIND::DIR,
                "/") {}

CFS_NODE *CFS_TREE::find(std::string path) {
  CFS_NODE *node = &root_node;
  std::vector<std::string> components = split_name(path);
  for (std::string child : components) {
    CFS_NODE *cnode = node->find_child(child);

    // Path not found!
    if (cnode == nullptr)
      return nullptr;
    node = cnode;
  }

  return node;
}

CFS_NODE *CFS_TREE::mkdir_p(std::string path) {
  CFS_NODE *node = &root_node;
  auto components = split_name(path);
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
