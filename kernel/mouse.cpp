#include "mouse.hpp"

#include "graphics.hpp"
#include "logger.hpp"

namespace {
const int kMouseCursorWidth = 15;
const int kMouseCursorHeight = 24;
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

void DrawMouseCursor(PixelWriter *pixel_writer, Vector2D<int> position) {
  for (int dy = 0; dy < kMouseCursorHeight; ++dy) {
    for (int dx = 0; dx < kMouseCursorWidth; ++dx) {
      if (mouse_cursor_shape[dy][dx] == '@') {
        pixel_writer->Write(position.x + dx, position.y + dy, {0, 0, 0});
      } else if (mouse_cursor_shape[dy][dx] == '.') {
        pixel_writer->Write(position.x + dx, position.y + dy, {255, 255, 255});
      }
    }
  }
}

void EraseMouseCursor(PixelWriter *pixel_writer, Vector2D<int> position,
                      PixelColor erase_color) {
  for (int dy = 0; dy < kMouseCursorHeight; ++dy) {
    for (int dx = 0; dx < kMouseCursorWidth; ++dx) {
      if (mouse_cursor_shape[dy][dx] != ' ') {
        pixel_writer->Write(position.x + dx, position.y + dy, erase_color);
      }
    }
  }
}
} // namespace

MouseCursor::MouseCursor(PixelWriter *writer, PixelColor erase_color,
                         Vector2D<int> initial_position)
    : pixel_writer(writer), erase_color(erase_color),
      position(initial_position) {
  DrawMouseCursor(pixel_writer, position);
}

void MouseCursor::MoveRelative(Vector2D<int> displacement) {
  Log(kDebug, "MoveRelative: %d, %d", displacement.x, displacement.y);
  EraseMouseCursor(pixel_writer, position, erase_color);
  position += displacement;
  DrawMouseCursor(pixel_writer, position);
}
