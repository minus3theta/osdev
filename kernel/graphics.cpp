#include "graphics.hpp"

uint8_t *PixelWriter::PixelAt(int x, int y) {
  return config.frame_buffer + 4 * (config.pixels_per_scan_line * y + x);
}

void BGRResv8BitPerColorPixelWriter::Write(int x, int y, const PixelColor &c) {
  auto p = PixelAt(x, y);
  p[0] = c.b;
  p[1] = c.g;
  p[2] = c.r;
}

void RGBResv8BitPerColorPixelWriter::Write(int x, int y, const PixelColor &c) {
  auto p = PixelAt(x, y);
  p[0] = c.r;
  p[1] = c.g;
  p[2] = c.b;
}
