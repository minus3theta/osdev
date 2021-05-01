#include "window.hpp"

#include "error.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "logger.hpp"

Window::Window(int width, int height, PixelFormat shadow_format)
    : width(width), height(height),
      data(height, std::vector(width, PixelColor{})) {
  FrameBufferConfig config;
  config.frame_buffer = nullptr;
  config.horizontal_resolution = width;
  config.vertical_resolution = height;
  config.pixel_format = shadow_format;

  if (auto err = shadow_buffer.Initialize(config)) {
    Log(kError, "failed to initialize shadow buffer: %s at %s:%d\n", err.Name(),
        err.File(), err.Line());
  }
}

void Window::DrawTo(FrameBuffer &dst, Vector2D<int> position) {
  if (!transparent_color) {
    dst.Copy(position, shadow_buffer);
    return;
  }

  const auto tc = transparent_color.value();
  auto &writer = dst.Writer();
  for (int y = 0; y < Height(); ++y) {
    for (int x = 0; x < Width(); ++x) {
      const auto p = Vector2D<int>{x, y};
      const auto c = At(p);
      if (c != tc) {
        writer.Write(position + p, c);
      }
    }
  }
}

void Window::SetTransparentColor(std::optional<PixelColor> c) {
  transparent_color = c;
}

Window::WindowWriter *Window::Writer() { return &writer; }

const PixelColor &Window::At(Vector2D<int> pos) const {
  return data[pos.y][pos.x];
}

void Window::Write(Vector2D<int> pos, PixelColor c) {
  data[pos.y][pos.x] = c;
  shadow_buffer.Writer().Write(pos, c);
}

int Window::Width() const { return width; }

int Window::Height() const { return height; }
