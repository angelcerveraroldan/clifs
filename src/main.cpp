#include <cerrno>
#include <iostream>

#define FUSE_USE_VERSION 31
#include <fuse3/fuse.h>

int main() {
  // Hello!
  std::cout << "Hello!" << std::endl;
}
