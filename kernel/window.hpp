#pragma once

#include <optional>
#include <vector>

#include "frame_buffer.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"

class Window {
public:
  class WindowWriter : public PixelWriter {
  public:
    WindowWriter(Window &window) : window{window} {}
    virtual void Write(Vector2D<int> pos, const PixelColor &c) override {
      window.Write(pos, c);
    }
    virtual int Width() const override { return window.Width(); }
    virtual int Height() const override { return window.Height(); }

  private:
    Window &window;
  };

  Window(int width, int height, PixelFormat shadow_format);
  ~Window() = default;
  Window(const Window &rhs) = delete;
  Window &operator=(const Window &rhs) = delete;

  void DrawTo(FrameBuffer &dst, Vector2D<int> position);
  void SetTransparentColor(std::optional<PixelColor> c);
  WindowWriter *Writer();

  const PixelColor &At(Vector2D<int> pos) const;
  void Write(Vector2D<int> pos, PixelColor c);

  int Width() const;
  int Height() const;

private:
  int width, height;
  std::vector<std::vector<PixelColor>> data;
  WindowWriter writer{*this};
  std::optional<PixelColor> transparent_color{std::nullopt};

  FrameBuffer shadow_buffer;
};
