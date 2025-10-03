#ifndef _CLIFS_STATE_H
#define _CLIFS_STATE_H

#include "clifs_tree.h"
#include "file_desc.h"

struct CFS_STATE {
  CFS_TREE tree;
  FD_TABLE file_desc;
};

#endif
