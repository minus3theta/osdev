#pragma once

#include "frame_buffer_config.hpp"

struct PixelColor {
  uint8_t r, g, b;
};

class PixelWriter {
public:
  PixelWriter(const FrameBufferConfig &config) : config(config) {}
  virtual ~PixelWriter() = default;
  virtual void Write(int x, int y, const PixelColor &c) = 0;

protected:
  uint8_t *PixelAt(int x, int y);

private:
  const FrameBufferConfig &config;
};

class RGBResv8BitPerColorPixelWriter : public PixelWriter {
public:
  using PixelWriter::PixelWriter;

  virtual void Write(int x, int y, const PixelColor &c) override;
};

class BGRResv8BitPerColorPixelWriter : public PixelWriter {
public:
  using PixelWriter::PixelWriter;

  virtual void Write(int x, int y, const PixelColor &c) override;
};

template <typename T> struct Vector2D {
  T x, y;

  template <typename U> Vector2D<T> &operator+=(const Vector2D<U> &rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }
};

void FillRectangle(PixelWriter &writer, const Vector2D<int> &pos,
                   const Vector2D<int> &size, const PixelColor &c);

void DrawRectangle(PixelWriter &writer, const Vector2D<int> &pos,
                   const Vector2D<int> &size, const PixelColor &c);
