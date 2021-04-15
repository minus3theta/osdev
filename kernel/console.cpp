#include <cstring>

#include "console.hpp"
#include "font.hpp"

Console::Console(PixelWriter &writer, const PixelColor &fg_color,
                 const PixelColor &bg_color)
    : writer(writer), fg_color(fg_color), bg_color(bg_color), buffer(),
      cursor_row(0), cursor_column(0) {}

void Console::PutString(const char *s) {
  while (*s) {
    if (*s == '\n') {
      Newline();
    } else if (cursor_column < kColumns - 1) {
      WriteAscii(writer, 8 * cursor_column, 16 * cursor_row, *s, fg_color);
      buffer[cursor_row][cursor_column] = *s;
      ++cursor_column;
    }
    ++s;
  }
}

void Console::Newline() {
  cursor_column = 0;
  if (cursor_row < kRows - 1) {
    ++cursor_row;
  } else {
    for (int y = 0; y < 16 * kRows; ++y) {
      for (int x = 0; x < 8 * kColumns; ++x) {
        writer.Write(x, y, bg_color);
      }
    }
    for (int row = 0; row < kRows - 1; ++row) {
      memcpy(buffer[row], buffer[row + 1], kColumns + 1);
      WriteString(writer, 0, 16 * row, buffer[row], fg_color);
    }
    memset(buffer[kRows - 1], 0, kColumns + 1);
  }
}
