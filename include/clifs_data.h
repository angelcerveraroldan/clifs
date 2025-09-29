#ifndef _DATA_CLIFS_H
#define _DATA_CLIFS_H

#include <string>

class CFS_DATA {
public:
  CFS_DATA(std::string root_dir);
  std::string get_absolute_path(std::string relative);

private:
  [[maybe_unused]] std::string root_dir;
};

#endif
