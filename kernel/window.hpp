#pragma once

#include <optional>
#include <string>
#include <vector>

#include "frame_buffer.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"

class Window : public PixelWriter {
public:
  Window(int width, int height, PixelFormat shadow_format);
  virtual ~Window() = default;
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

  virtual void Activate() {}
  virtual void Deactivate() {}

private:
  int width, height;
  std::vector<std::vector<PixelColor>> data;
  std::optional<PixelColor> transparent_color{std::nullopt};

  FrameBuffer shadow_buffer;
};

void DrawWindow(PixelWriter &writer, const char *title);
void DrawTextbox(PixelWriter &writer, Vector2D<int> pos, Vector2D<int> size);
void DrawWindowTitle(PixelWriter &writer, const char* title, bool active);

class ToplevelWindow : public Window {
public:
  static constexpr Vector2D<int> kTopLeftMargin{4, 24};
  static constexpr Vector2D<int> kBottomRightMargin{4, 4};

  class InnerAreaWriter : public PixelWriter {
  public:
    InnerAreaWriter(ToplevelWindow &window) : window{window} {}
    virtual void Write(Vector2D<int> pos, const PixelColor &c) override {
      window.Write(pos + kTopLeftMargin, c);
    }
    virtual int Width() const override {
      return window.Width() - kTopLeftMargin.x - kBottomRightMargin.x;
    }
    virtual int Height() const override {
      return window.Height() - kTopLeftMargin.y - kBottomRightMargin.y;
    }

  private:
    ToplevelWindow &window;
  };

  ToplevelWindow(int width, int height, PixelFormat shadow_format,
                 const std::string &title);

  virtual void Activate() override;
  virtual void Deactivate() override;

  InnerAreaWriter *InnerWriter() { return &inner_writer; }
  Vector2D<int> InnerSize() const;

private:
  std::string title;
  InnerAreaWriter inner_writer{*this};
};
