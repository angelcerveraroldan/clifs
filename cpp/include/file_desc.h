#ifndef _CLIFS_FILE_DESCRIPTOR_H
#define _CLIFS_FILE_DESCRIPTOR_H

#include "clifs_tree.h"
#include <cstdint>
#include <optional>
#include <vector>

struct FileHandle {
  CFS_NODE *node;
  int flags;
};

typedef std::optional<FileHandle> MaybeFileHandle;

class FD_TABLE {
public:
  // Open a new file -- needs to be stored in the file handle list
  uint64_t open(CFS_NODE *n, int flags);
  MaybeFileHandle get(uint64_t) const;
  FileHandle *get(uint64_t);
  void close(uint64_t);

private:
  std::vector<MaybeFileHandle> handles;
  std::vector<uint64_t> free_indices;
};

#endif
