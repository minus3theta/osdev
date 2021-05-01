#include "window.hpp"

#include "graphics.hpp"

Window::Window(int width, int height)
    : width(width), height(height),
      data(height, std::vector(width, PixelColor{})) {}

void Window::DrawTo(PixelWriter &writer, Vector2D<int> position) {
  if (!transparent_color) {
    for (int y = 0; y < Height(); ++y) {
      for (int x = 0; x < Width(); ++x) {
        writer.Write(position.x + x, position.y + y, At(x, y));
      }
    }
    return;
  }

  const auto tc = transparent_color.value();
  for (int y = 0; y < Height(); ++y) {
    for (int x = 0; x < Width(); ++x) {
      const auto c = At(x, y);
      if (c != tc) {
        writer.Write(position.x + x, position.y + y, At(x, y));
      }
    }
  }
}

void Window::SetTransparentColor(std::optional<PixelColor> c) {
  transparent_color = c;
}

Window::WindowWriter *Window::Writer() { return &writer; }

PixelColor &Window::At(int x, int y) { return data[y][x]; }

const PixelColor &Window::At(int x, int y) const { return data[y][x]; }

int Window::Width() const { return width; }

int Window::Height() const { return height; }
