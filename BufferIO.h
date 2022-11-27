#ifndef MYFUSE__BUFFERIO_H_
#define MYFUSE__BUFFERIO_H_
#include <cstddef>

class BufferIO {
 public:
  BufferIO();
  ~BufferIO();
  BufferIO(const BufferIO &) = delete;
  int Write(const char *const data, size_t size, unsigned long offset);
  int Read(char *const data, size_t size, unsigned long offset);
  void Truncate(size_t size);
  size_t Size() const;
  size_t BufferSize() const;
 protected:
  void realloc(size_t required_size);
 private:
  size_t size_, buffer_size_;
  char *buffer_;
};

#endif //MYFUSE__BUFFERIO_H_
