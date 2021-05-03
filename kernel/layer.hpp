#pragma once

#include <memory>
#include <vector>

#include "frame_buffer.hpp"
#include "graphics.hpp"
#include "window.hpp"

class Layer {
public:
  Layer(unsigned int id = 0);
  unsigned int ID() const;

  Layer &SetWindow(const std::shared_ptr<Window> &window);
  std::shared_ptr<Window> GetWindow() const;

  Layer &Move(Vector2D<int> pos);
  Layer &MoveRelative(Vector2D<int> pos_diff);
  Layer &SetDraggable(bool draggable);

  void DrawTo(FrameBuffer &screen, const Rectangle<int> &area) const;

  Vector2D<int> GetPosition() const;
  bool IsDraggable() const;

private:
  unsigned int id;
  Vector2D<int> pos;
  std::shared_ptr<Window> window;
  bool draggable{false};
};

class LayerManager {
public:
  void SetWriter(FrameBuffer *screen);
  Layer &NewLayer();

  void Draw(const Rectangle<int> &area) const;
  void Draw(unsigned int id) const;
  void Move(unsigned int id, Vector2D<int> new_position);
  void MoveRelative(unsigned int id, Vector2D<int> pos_diff);

  void UpDown(unsigned int id, int new_height);
  void Hide(unsigned int id);

  Layer *FindLayerByPosition(Vector2D<int> pos, unsigned int exclude_id) const;

private:
  FrameBuffer *screen{nullptr};
  mutable FrameBuffer back_buffer;

  std::vector<std::unique_ptr<Layer>> layers;
  std::vector<Layer *> layer_stack;
  unsigned int latest_id{0};

  Layer *FindLayer(unsigned int id);
};

inline LayerManager *layer_manager;
