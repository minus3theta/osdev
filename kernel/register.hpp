#pragma once

#include <cstddef>
#include <cstdint>

template <typename T> struct ArrayLength {};

template <typename T, size_t N> struct ArrayLength<T[N]> {
  static const size_t value = N;
};

template <typename T> class MemMapRegister {
public:
  T Read() const {
    T tmp;
    for (size_t i = 0; i < len; ++i) {
      tmp.data[i] = value.data[i];
    }
    return tmp;
  }

  void Write(const T &value) {
    for (size_t i = 0; i < len; ++i) {
      this->value.data[i] = value.data[i];
    }
  }

private:
  volatile T value;
  static const size_t len = ArrayLength<decltype(T::data)>::value;
};

template <typename T> struct DefaultBitmap {
  T data[1];

  DefaultBitmap &operator=(const T &value) { data[0] = value; }
  operator T() const { return data[0]; }
};

template <typename T> class ArrayWrapper {
public:
  using ValueType = T;
  using Iterator = ValueType *;
  using ConstIterator = const ValueType *;

  ArrayWrapper(uintptr_t array_base_addr, size_t size)
      : array(reinterpret_cast<ValueType *>(array_base_addr)), size(size) {}

  size_t Size() const { return size; }

  Iterator begin() { return array; }
  Iterator end() { return array + size; }
  ConstIterator cbegin() const { return array; }
  ConstIterator cend() const { return array + size; }

  ValueType &operator[](size_t index) { return array[index]; }

private:
  ValueType *const array;
  const size_t size;
};
