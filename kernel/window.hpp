#pragma once

#include <optional>
#include <vector>

#include "graphics.hpp"

class Window {
public:
  class WindowWriter : public PixelWriter {
  public:
    WindowWriter(Window &window) : window{window} {}
    virtual void Write(int x, int y, const PixelColor &c) override {
      window.At(x, y) = c;
    }
    virtual int Width() const override { return window.Width(); }
    virtual int Height() const override { return window.Height(); }

  private:
    Window &window;
  };

  Window(int width, int height);
  ~Window() = default;
  Window(const Window &rhs) = delete;
  Window &operator=(const Window &rhs) = delete;

  void DrawTo(PixelWriter &writer, Vector2D<int> position);
  void SetTransparentColor(std::optional<PixelColor> c);
  WindowWriter *Writer();

  PixelColor &At(int x, int y);
  const PixelColor &At(int x, int y) const;

  int Width() const;
  int Height() const;

private:
  int width, height;
  std::vector<std::vector<PixelColor>> data;
  WindowWriter writer{*this};
  std::optional<PixelColor> transparent_color{std::nullopt};
};
