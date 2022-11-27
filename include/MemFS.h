#ifndef MYFUSE__MEMFS_H_
#define MYFUSE__MEMFS_H_
#include "BufferIO.h"
#include "FileSystem.h"
#include <map>
#include <memory>
#include <string>


class MemFileSystem;

using FileId = unsigned long long;


class MemFile : public RegularFile {
 public:
  MemFile(int owner, int mode, FileId id);
  MemFile(const MemFile &) = delete;
  int Write(const char * data, size_t size, unsigned long offset) override;
  int Read(char * data, size_t size, unsigned long offset) override;
  [[nodiscard]] size_t Size() const override;
  void Truncate(size_t size) override;
  ~MemFile() noexcept override;
 private:
  FileId id_;
  BufferIO data_;
  friend class MemDirectory;
};


class MemDirectory : public Directory {
 public:
  MemDirectory(int owner, int mode, Directory *parent, MemFileSystem &fs, FileId id);
  File *GetFile(const std::string &name) override;
  template <typename T>
  T *GetFile(const std::string &name) {
    File *file = GetFile(name);
    if (file == nullptr) {
      return nullptr;
    }
    return file->template Cast<T>();
  }
  [[nodiscard]] std::map<std::string, const std::reference_wrapper<File>> List() const override;
  bool CreateFile(const std::string &name, int mode) override;
  bool CreateDirectory(const std::string &name, int mode) override;
  bool Exists(const std::string &name) override;
  bool RemoveFile(const std::string &name) override;
  bool RemoveDirectory(const std::string &name) override;
  ~MemDirectory() noexcept override;
 private:
  MemFileSystem &fs_;
  Directory *parent_;
  FileId id_;
  std::map<std::string, std::reference_wrapper<File>> files_;
  friend class MemFileSystem;
};

class MemFileSystem : public FileSystem {
 public:
  MemFileSystem();
  ~MemFileSystem() noexcept override;
  template<typename T, typename...Args>
  T &Insert(Args &&...args) {
    FileId id = id_++;
    files_.emplace(id, std::make_unique<T>(std::forward<Args>(args)..., id));
    return *static_cast<T *>(files_.find(id)->second.get());
  }
  void Remove(FileId id);
  Directory &Root() override;
  File *GetFileFromPath(const char *path);
  template <typename T>
  T *GetFileFromPath(const char *path) {
    auto *file = GetFileFromPath(path);
    if (file == nullptr) {
      return nullptr;
    }
    return file->template Cast<T>();
  }
  bool Link(const std::string &from, const std::string &to) override;

 private:
  FileId id_;
  std::map<FileId, std::unique_ptr<File>> files_;
};

#endif //MYFUSE__MEMFS_H_
