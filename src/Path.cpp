#include "../include/Path.h"


std::tuple<std::string, std::string> Split(const std::string &path) {
  auto e = path.end();
  do {
    e--;
  } while (*e != '/');
  std::string parent(path.begin(), e);
  if (parent.empty()) {
    parent = "/";
  }
  e++;
  std::string name(e, path.end());
  return std::make_tuple(parent, name);
}
