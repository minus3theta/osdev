#pragma once

#include <optional>
#include <vector>

#include "frame_buffer.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"

class Window : public PixelWriter {
public:
  Window(int width, int height, PixelFormat shadow_format);
  ~Window() = default;
  Window(const Window &rhs) = delete;
  Window &operator=(const Window &rhs) = delete;

  void DrawTo(FrameBuffer &dst, Vector2D<int> pos, const Rectangle<int> &area);
  void SetTransparentColor(std::optional<PixelColor> c);

  const PixelColor &At(Vector2D<int> pos) const;
  virtual void Write(Vector2D<int> pos, const PixelColor &c) override;

  virtual int Width() const override;
  virtual int Height() const override;
  Vector2D<int> Size() const;

  void Move(Vector2D<int> dst_pos, const Rectangle<int> &src);

private:
  int width, height;
  std::vector<std::vector<PixelColor>> data;
  std::optional<PixelColor> transparent_color{std::nullopt};

  FrameBuffer shadow_buffer;
};

void DrawWindow(PixelWriter &writer, const char *title);
void DrawTextbox(PixelWriter &writer, Vector2D<int> pos, Vector2D<int> size);
