#include "file_desc.h"
#include "clifs_tree.h"
#include <cstdint>
#include <optional>

uint64_t FD_TABLE::open(CFS_NODE *node, int flags) {
  FileHandle fh = FileHandle{node, flags};
  // Find the first free spot
  if (!free_indices.empty()) {
    uint64_t ffree = free_indices.back();
    free_indices.pop_back();
    handles[ffree] = fh;
    return ffree;
  }
  // No empty spot, we will add at the end
  handles.emplace_back(fh);
  return handles.size() - 1;
}

MaybeFileHandle FD_TABLE::get(uint64_t id) const {
  if (id >= handles.size())
    return std::nullopt;
  return handles[id];
}

FileHandle *FD_TABLE::get(uint64_t id) {
  if (id >= handles.size() || !handles[id].has_value())
    return nullptr;
  return &handles[id].value();
}

void FD_TABLE::close(uint64_t id) {
  // If it does not have a vlue, then the id is already in free, and we dont
  // need to do anything.
  if (id >= handles.size() || !handles[id].has_value())
    return;

  handles[id].reset();
  free_indices.emplace_back(id);
  return;
}
