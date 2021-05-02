#include "window.hpp"

#include "error.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "logger.hpp"
#include <algorithm>

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
  for (int y = std::max(0, 0 - position.y);
       y < std::min(Height(), writer.Height() - position.y); ++y) {
    for (int x = std::max(0, 0 - position.x);
         x < std::min(Width(), writer.Width() - position.x); ++x) {
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

const PixelColor &Window::At(Vector2D<int> pos) const {
  return data[pos.y][pos.x];
}

void Window::Write(Vector2D<int> pos, const PixelColor &c) {
  data[pos.y][pos.x] = c;
  shadow_buffer.Writer().Write(pos, c);
}

int Window::Width() const { return width; }

int Window::Height() const { return height; }

void Window::Move(Vector2D<int> dst_pos, const Rectangle<int> &src) {
  shadow_buffer.Move(dst_pos, src);
}
