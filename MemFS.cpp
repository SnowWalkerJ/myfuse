#include "MemFS.h"
#include "Path.h"
#include <unistd.h>

MemFile::MemFile(int owner, int mode, FileId id) : RegularFile(owner, mode), id_(id) {}

size_t MemFile::Size() const {
  return data_.Size();
}

int MemFile::Read(char *const data, size_t size, unsigned long offset) {
  return data_.Read(data, size, offset);
}

int MemFile::Write(const char *const data, size_t size, unsigned long offset) {
  return data_.Write(data, size, offset);
}

void MemFile::Truncate(size_t size) {
  data_.Truncate(size);
}

MemFile::~MemFile() noexcept = default;

MemDirectory::MemDirectory(int owner, int mode, Directory *parent, MemFileSystem &fs, FileId id) :
    Directory(owner, mode), parent_(parent), fs_(fs), id_(id) {
  Inc();
  files_.emplace(".", *this);
  if (parent != nullptr) {
    parent->Inc();
    files_.emplace("..", *parent);
  }
}

bool MemDirectory::CreateDirectory(const std::string &name, int mode) {
  if (files_.count(name) > 0)
    return false;

  auto &directory = fs_.Insert<MemDirectory>(getuid(), mode, this, fs_);
  files_.emplace(name, directory);
  return true;
}

bool MemDirectory::CreateFile(const std::string &name, int mode) {
  if (files_.count(name) > 0)
    return false;

  auto &file = fs_.Insert<MemFile>(getuid(), mode);
  files_.emplace(name, file);
  return true;
}

bool MemDirectory::RemoveFile(const std::string &name) {
  if (!Exists(name)) {
    return false;
  }
  auto *file = this->GetFile<MemFile>(name);
  if (file == nullptr) {
    return false;
  }
  files_.erase(name);
  file->Dec();
  if (file->HardLinks() == 0) {
    fs_.Remove(file->id_);
  }
  return true;
}

bool MemDirectory::RemoveDirectory(const std::string &name) {
  if (!Exists(name)) {
    return false;
  }
  auto *file = GetFile<MemDirectory>(name);
  if (file == nullptr) {
    return false;
  }
  files_.erase(name);
  file->Dec();
  if (file->HardLinks() == 1) {
    fs_.Remove(file->id_);
  }
  return true;
}

bool MemDirectory::Exists(const std::string &name) {
  return files_.count(name) > 0;
}

File *MemDirectory::GetFile(const std::string &name) {
  auto rec = files_.find(name);
  if (rec == files_.end()) {
    return nullptr;
  }
  return &rec->second.get();
}

std::map<std::string, const std::reference_wrapper<File>> MemDirectory::List() const {
  return {files_.begin(), files_.end()};
}

MemDirectory::~MemDirectory() noexcept = default;


MemFileSystem::MemFileSystem() {
  id_ = 0;
  Insert<MemDirectory>(getuid(), 0755, nullptr, *this);
}

MemFileSystem::~MemFileSystem() noexcept = default;

void MemFileSystem::Remove(FileId id) {
  auto rec = files_.find(id);
  if (rec != files_.end()) {
    files_.erase(rec);
  }
}

Directory &MemFileSystem::Root() {
  return *dynamic_cast<Directory *>(files_.find(0)->second.get());
}
File *MemFileSystem::GetFileFromPath(const char *path) {
  assert(path[0] == '/');
  File *file = &Root();
  const char *s = &path[0], *e = &path[0];
  while (true) {
    if (*e == 0 || file == nullptr) {
      return file;
    }
    auto *dir = dynamic_cast<MemDirectory *>(file);
    if (dir == nullptr) {
      return nullptr;
    }
    e++;
    while (*e != 0 && *e != '/')
      e++;
    if (e == s + 1) {
      return file;
    }
    std::string name(s + 1, e);
    file = dir->GetFile(name);
    s = e;
  }
}

bool MemFileSystem::Link(const std::string &from, const std::string &to) {
  auto *src = GetFileFromPath(from.c_str());
  auto [parent, name] = Split(std::string(to));
  auto *dst_parent = GetFileFromPath<MemDirectory>(parent.c_str());
  if (dst_parent == nullptr || dst_parent->Exists(name)) {
    return false;
  }
  dst_parent->files_.emplace(name, *src);
  src->Inc();
  return true;
}
