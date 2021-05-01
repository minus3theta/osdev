#include "mouse.hpp"

#include "graphics.hpp"
#include "logger.hpp"

namespace {
const char mouse_cursor_shape[kMouseCursorHeight][kMouseCursorWidth + 1] = {
    // clang-format off
  "@              ",
  "@@             ",
  "@.@            ",
  "@..@           ",
  "@...@          ",
  "@....@         ",
  "@.....@        ",
  "@......@       ",
  "@.......@      ",
  "@........@     ",
  "@.........@    ",
  "@..........@   ",
  "@...........@  ",
  "@............@ ",
  "@......@@@@@@@@",
  "@......@       ",
  "@....@@.@      ",
  "@...@ @.@      ",
  "@..@   @.@     ",
  "@.@    @.@     ",
  "@@      @.@    ",
  "@       @.@    ",
  "         @.@   ",
  "         @@@   ",
    // clang-format on
};
}

void DrawMouseCursor(PixelWriter *pixel_writer, Vector2D<int> position) {
  for (int dy = 0; dy < kMouseCursorHeight; ++dy) {
    for (int dx = 0; dx < kMouseCursorWidth; ++dx) {
      const auto p = Vector2D<int>{dx, dy};
      if (mouse_cursor_shape[dy][dx] == '@') {
        pixel_writer->Write(position + p, {0, 0, 0});
      } else if (mouse_cursor_shape[dy][dx] == '.') {
        pixel_writer->Write(position + p, {255, 255, 255});
      } else {
        pixel_writer->Write(position + p, kMouseTransparentColor);
      }
    }
  }
}
