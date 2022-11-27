#define FUSE_USE_VERSION 30
#define _FILE_OFFSET_BITS 64
#include <iostream>
#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include "MemFS.h"
#include "Path.h"

extern "C" {

  MemFileSystem fs;

static int do_getattr(const char *path, struct stat *st) {
  std::cout << "[getattr] " << path << std::endl;
  // GNU's definitions of the attributes (http://www.gnu.org/software/libc/manual/html_node/Attribute-Meanings.html):
  // 		st_uid: 	The user ID of the file’s owner.
  //		st_gid: 	The group ID of the file.
  //		st_atime: 	This is the last access time for the file.
  //		st_mtime: 	This is the time of the last modification to the contents of the file.
  //		st_mode: 	Specifies the mode of the file. This includes file type information (see Testing File Type) and the file permission bits (see Permission Bits).
  //		st_nlink: 	The number of hard links to the file. This count keeps track of how many directories have entries for this file. If the count is ever decremented to zero, then the file itself is discarded as soon
  //						as no process still holds it open. Symbolic links are not counted in the total.
  //		st_size:	This specifies the size of a regular file in bytes. For files that are really devices this field isn’t usually meaningful. For symbolic links this specifies the length of the file name the link refers to.

  auto *file = fs.GetFileFromPath(path);
  if (file == nullptr) {
    std::cout << "    not exists" << std::endl;
    return -ENOENT;
  }
  st->st_uid = getuid();
  st->st_gid = getgid();
  st->st_atime = file->ATime();
  st->st_mtime = file->MTime();
  st->st_mode = (file->Cast<RegularFile>() == nullptr ? S_IFDIR : S_IFREG) | file->Mode();
  st->st_nlink = file->HardLinks();
  std::cout
      << "uid=" << st->st_uid
      << ",gid=" << st->st_gid
      << ",atime=" << st->st_atime
      << ",mode=" << std::oct << st->st_mode << std::dec
      << ",nlink=" << st->st_nlink << std::endl;
  auto *memfile = file->Cast<MemFile>();
  if (memfile != nullptr) {
    st->st_size = static_cast<long long>(memfile->Size());
  }

  return 0;
}

static int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
  printf("--> Getting The List of Files of %s\n", path);

  filler(buffer, ".", NULL, 0); // Current Directory
  auto *dir = fs.GetFileFromPath<MemDirectory>(path);
  if (dir == nullptr) {
    return -1;
  }
  for (auto &&iter : dir->List()) {
    filler(buffer, iter.first.c_str(), NULL, 0);
  }

  return 0;
}

static int do_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
  printf("--> Trying to read %s, %lld, %zu\n", path, offset, size);

  auto *file = fs.GetFileFromPath(path)->Cast<MemFile>();

  size_t count = file->Read(buffer, size, offset);

  return static_cast<int>(file->Size() - offset - count);
}

static int do_write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info) {
  std::cout << "[write] " << path << std::endl;
  auto *file = fs.GetFileFromPath(path);
  if (file == nullptr) {
    return -1;
  }
  auto wfile = file->Cast<MemFile>();
  wfile->Write(buffer, size, offset);
  return static_cast<int>(size);
}

static int do_mkdir(const char *path, mode_t mode){
  std::cout << "[mkdir] " << path << ":" << std::oct << mode << std::dec << std::endl;
  auto [parent, name] = Split(std::string(path));
  auto *path_obj = fs.GetFileFromPath<MemDirectory>(parent.c_str());
  if (path_obj == nullptr) {
    return -1;
  }
  path_obj->CreateDirectory(name, mode);
  return 0;
}

static int do_mknod(const char *path, mode_t mode, dev_t rdev) {
  std::cout << "[mknod] " << path << ":" << std::oct << mode << std::dec << std::endl;
  auto [parent, name] = Split(std::string(path));
  std::cout << "parent=" << parent << ",name=" << name << std::endl;
  auto *path_obj = fs.GetFileFromPath<MemDirectory>(parent.c_str());
  if (path_obj == nullptr) {
    return -1;
  }
  path_obj->CreateFile(name, mode);
  return 0;
}

int do_truncate(const char *path, off_t offset) {
  std::cout << "[truncate] " << path << '(' << offset << ')' << std::endl;
  auto *file = fs.GetFileFromPath<MemFile>(path);
  if (file == nullptr) {
    return -1;
  }
  file->Truncate(offset);
  return 0;
}

int do_link(const char *from, const char *to) {
  return !fs.Link(std::string(from), std::string(to));
}

int do_unlink(const char *path) {
  std::cout << "[unlink] " << path << std::endl;
  auto [parent, name] = Split(std::string(path));
  auto *parent_dir = fs.GetFileFromPath<MemDirectory>(parent.c_str());
  if (parent_dir == nullptr) {
    return -1;
  }
  parent_dir->RemoveFile(name);
  return 0;
}

int do_rmdir(const char *path) {
  std::cout << "[rmdir] " << path << std::endl;
  auto [parent, name] = Split(std::string(path));
  auto *parent_dir = fs.GetFileFromPath<MemDirectory>(parent.c_str());
  auto *dir = fs.GetFileFromPath<MemDirectory>(path);
  if (dir == nullptr || dir->List().size() > 2) {
    return -1;
  }
  parent_dir->RemoveDirectory(name);
  return 0;
}

int do_utimens(const char *path, const timespec time[2]) {
  auto *file = fs.GetFileFromPath(path);
  if (file == nullptr) {
    return -1;
  }
  file->UpdateTime(time[0], time[1]);
  return 0;
}

static struct fuse_operations operations = {
    .getattr    = do_getattr,
    .mknod      = do_mknod,
    .mkdir      = do_mkdir,
    .unlink     = do_unlink,
    .rmdir      = do_rmdir,
    .link       = do_link,
    .truncate   = do_truncate,
    .read       = do_read,
    .write      = do_write,
    .readdir    = do_readdir,
    .utimens    = do_utimens,
};

int main(int argc, char *argv[]) {
  return fuse_main(argc, argv, &operations, nullptr);
}
};
