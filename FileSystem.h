#ifndef MYFUSE__FILESYSTEM_H_
#define MYFUSE__FILESYSTEM_H_
#include <functional>
#include <map>
#include <string>
#include <ctime>
#include <vector>

class File;
class Directory;
class RegularFile;


class FileSystem {
 public:
  virtual Directory &Root() = 0;
  virtual bool Link(const std::string &from, const std::string &to) = 0;
  virtual ~FileSystem() = default;
};

class File {
 public:
  File(int owner, int mode) : owner_(owner), mode_(mode) {
    Inc();
    atime_ = mtime_ = ctime_ = time(nullptr);
  }
  [[nodiscard]] size_t HardLinks() const { return hard_links_; };
  [[nodiscard]] int Owner() const { return owner_; };
  [[nodiscard]] int Mode() const { return mode_; };
  [[nodiscard]] time_t ATime() const { return atime_; }
  [[nodiscard]] time_t MTime() const { return mtime_; }
  [[nodiscard]] time_t CTime() const { return ctime_; }
  virtual void UpdateTime(timespec atime, timespec mtime) {
    atime_ = atime.tv_sec;
    mtime_ = mtime.tv_sec;
  }
  virtual ~File() noexcept = default;;
  void Inc() { hard_links_++; }
  void Dec() { hard_links_--; }
  template <typename T>
  T *Cast() {
    return dynamic_cast<T *>(this);
  }

 private:
  int owner_;
  int mode_;
  time_t atime_, mtime_, ctime_;
  size_t hard_links_ = 0;
};

class Directory : public File {
 public:
  using File::File;
  virtual std::map<std::string, const std::reference_wrapper<File>> List() const = 0;
  virtual File *GetFile(const std::string &name) = 0;
  virtual bool CreateFile(const std::string &name, int mode) = 0;
  virtual bool CreateDirectory(const std::string &name, int mode) = 0;
  virtual bool RemoveFile(const std::string &name) = 0;
  virtual bool RemoveDirectory(const std::string &name) = 0;
  virtual bool Exists(const std::string &name) = 0;
  template <typename T>
  T *GetFile(const std::string &name) {
    auto *file = GetFile(name);
    if (file == nullptr) {
      return nullptr;
    }
    return file->template Cast<T>();
  }
};

class RegularFile : public File {
 public:
  using File::File;
  virtual int Write(const char * data, size_t size, unsigned long offset) = 0;
  virtual int Read(char * data, size_t size, unsigned long offset) = 0;
  [[nodiscard]] virtual size_t Size() const = 0;
  virtual void Truncate(size_t size) = 0;
};

#endif //MYFUSE__FILESYSTEM_H_
