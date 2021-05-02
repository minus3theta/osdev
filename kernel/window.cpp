#include "window.hpp"

#include "error.hpp"
#include "font.hpp"
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

namespace {
const int kCloseButtonWidth = 16;
const int kCloseButtonHeight = 14;
const char close_button[kCloseButtonHeight][kCloseButtonWidth + 1] = {
    // clang-format off
  "...............@",
  ".:::::::::::::$@",
  ".:::::::::::::$@",
  ".:::@@::::@@::$@",
  ".::::@@::@@:::$@",
  ".:::::@@@@::::$@",
  ".::::::@@:::::$@",
  ".:::::@@@@::::$@",
  ".::::@@::@@:::$@",
  ".:::@@::::@@::$@",
  ".:::::::::::::$@",
  ".:::::::::::::$@",
  ".$$$$$$$$$$$$$$@",
  "@@@@@@@@@@@@@@@@",
    // clang-format on
};

constexpr PixelColor ToColor(uint32_t c) {
  return {static_cast<uint8_t>((c >> 16) & 0xff),
          static_cast<uint8_t>((c >> 8) & 0xff),
          static_cast<uint8_t>(c & 0xff)};
}

PixelColor CloseButtonColor(char c) {
  uint32_t color = 0xffffff;
  switch (c) {
  case '@':
    color = 0x000000;
    break;
  case '$':
    color = 0x848484;
    break;
  case ':':
    color = 0xc6c6c6;
    break;
  }
  return ToColor(color);
}
} // namespace

void DrawWindow(PixelWriter &writer, const char *title) {
  auto fill_rect = [&writer](Vector2D<int> pos, Vector2D<int> size,
                             uint32_t c) {
    FillRectangle(writer, pos, size, ToColor(c));
  };
  const auto win_w = writer.Width();
  const auto win_h = writer.Height();

  fill_rect({0, 0}, {win_w, 1}, 0xc6c6c6);
  fill_rect({1, 1}, {win_w - 2, 1}, 0xffffff);
  fill_rect({0, 0}, {1, win_h}, 0xc6c6c6);
  fill_rect({1, 1}, {1, win_h - 2}, 0xffffff);
  fill_rect({win_w - 2, 1}, {1, win_h - 2}, 0x848484);
  fill_rect({win_w - 1, 0}, {1, win_h}, 0x000000);
  fill_rect({2, 2}, {win_w - 4, win_h - 4}, 0xc6c6c6);
  fill_rect({3, 3}, {win_w - 6, 18}, 0x000084);
  fill_rect({1, win_h - 2}, {win_w - 2, 1}, 0x848484);
  fill_rect({0, win_h - 1}, {win_w, 1}, 0x000000);

  WriteString(writer, {24, 4}, title, ToColor(0xffffff));

  for (int y = 0; y < kCloseButtonHeight; ++y) {
    for (int x = 0; x < kCloseButtonWidth; ++x) {
      PixelColor c = CloseButtonColor(close_button[y][x]);
      writer.Write({win_w - 5 - kCloseButtonWidth + x, 5 + y}, c);
    }
  }
}
