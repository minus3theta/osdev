#pragma once

#include <memory>
#include <vector>

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

  void DrawTo(PixelWriter &writer) const;

private:
  unsigned int id;
  Vector2D<int> pos;
  std::shared_ptr<Window> window;
};

class LayerManager {
public:
  void SetWriter(PixelWriter *writer);
  Layer &NewLayer();

  void Draw() const;
  void Move(unsigned int id, Vector2D<int> new_position);
  void MoveRelative(unsigned int id, Vector2D<int> pos_diff);

  void UpDown(unsigned int id, int new_height);
  void Hide(unsigned int id);

private:
  PixelWriter *writer{nullptr};
  std::vector<std::unique_ptr<Layer>> layers;
  std::vector<Layer *> layer_stack;
  unsigned int latest_id{0};

  Layer *FindLayer(unsigned int id);
};

inline LayerManager *layer_manager;
