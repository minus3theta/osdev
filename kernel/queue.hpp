#pragma once

#include <array>
#include <cstddef>

#include "error.hpp"

template <typename T> class ArrayQueue {
public:
  template <size_t N> ArrayQueue(std::array<T, N> &buf);
  ArrayQueue(T *buf, size_t size);
  Error Push(const T &value);
  Error Pop();
  size_t Count() const;
  size_t Capacity() const;
  const T &Front() const;

private:
  T *data;
  size_t read_pos, write_pos, count;
  const size_t capacity;
};

template <typename T>
template <size_t N>
ArrayQueue<T>::ArrayQueue(std::array<T, N> &buf) : ArrayQueue(buf.data(), N) {}

template <typename T>
ArrayQueue<T>::ArrayQueue(T *buf, size_t size)
    : data(buf), read_pos(0), write_pos(0), count(0), capacity(size) {}

template <typename T> Error ArrayQueue<T>::Push(const T &value) {
  if (count == capacity) {
    return MAKE_ERROR(Error::kFull);
  }

  data[write_pos] = value;
  ++count;
  ++write_pos;
  if (write_pos == capacity) {
    write_pos = 0;
  }
  return MAKE_ERROR(Error::kSuccess);
}

template <typename T> Error ArrayQueue<T>::Pop() {
  if (count == 0) {
    return MAKE_ERROR(Error::kEmpty);
  }

  --count;
  ++read_pos;
  if (read_pos == capacity) {
    read_pos = 0;
  }
  return MAKE_ERROR(Error::kSuccess);
}

template <typename T> size_t ArrayQueue<T>::Count() const { return count; }

template <typename T> size_t ArrayQueue<T>::Capacity() const {
  return capacity;
}

template <typename T> const T &ArrayQueue<T>::Front() const {
  return data[read_pos];
}
