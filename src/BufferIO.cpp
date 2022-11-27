#include "../include/BufferIO.h"
#include <cstring>

BufferIO::BufferIO() {
  size_ = 0;
  buffer_size_ = 0;
  buffer_ = nullptr;
}

BufferIO::~BufferIO() {
  if (buffer_ != nullptr) {
    delete[] buffer_;
  }
}

size_t BufferIO::Size() const {
  return size_;
}

size_t BufferIO::BufferSize() const {
  return buffer_size_;
}


int BufferIO::Write(const char *const data, size_t size, unsigned long offset) {
  if (offset + size > buffer_size_) {
    realloc(offset + size);
  }
  std::memcpy(buffer_ + offset, data, size);
  size_ = size + offset > size_ ? size + offset : size_;
  return size;
}

int BufferIO::Read(char *const data, size_t size, unsigned long offset) {
  if (size + offset > Size()) {
    size = Size() - offset;
  }
  std::memcpy(data, buffer_ + offset, size);
  return size;
}

void BufferIO::Truncate(size_t size) {
  size_ = size;
}

void BufferIO::realloc(size_t required_size) {
  const size_t UNIT = 4 * 1024;
  size_t size = ((required_size - 1) / UNIT + 1) * UNIT;
  char *new_buffer = new char[size];
  std::memcpy(new_buffer, buffer_, size_);
  delete[] buffer_;
  buffer_ = new_buffer;
  buffer_size_ = size;
}