#include <iostream>
#include "../include/MemFS.h"


int main() {
  MemFileSystem fs;
  auto &root = fs.Root();
  root.CreateFile("a.txt", 0644);
  root.GetFile<MemFile>("a.txt")->Write("hello", 6, 0);
  fs.Link("/a.txt", "/b.txt");
  auto *a = root.GetFile<MemFile>("a.txt");
  auto *b = root.GetFile<MemFile>("b.txt");
  assert(a == b);
  assert(b->Size() == 6);
  char buf[6];
  b->Read(buf, 6, 0);
  std::cout << buf << std::endl;
  assert(strcmp(buf, "hello") == 0);
  return 0;
}