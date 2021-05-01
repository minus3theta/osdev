#pragma once

#include "graphics.hpp"

class Console {
public:
  static const int kRows = 25, kColumns = 80;

  Console(const PixelColor &fg_color, const PixelColor &bg_color);
  void PutString(const char *s);
  void SetWriter(PixelWriter *writer);

private:
  void Newline();
  void Refresh();

  PixelWriter *writer;
  const PixelColor fg_color, bg_color;
  char buffer[kRows][kColumns + 1];
  int cursor_row, cursor_column;
};
