#include "layer.hpp"
#include "frame_buffer.hpp"
#include <algorithm>
#include <memory>

Layer::Layer(unsigned int id) : id(id) {}

Layer &Layer::SetWindow(const std::shared_ptr<Window> &window) {
  this->window = window;
  return *this;
}

std::shared_ptr<Window> Layer::GetWindow() const { return window; }

unsigned int Layer::ID() const { return id; }

Layer &Layer::Move(Vector2D<int> pos) {
  this->pos = pos;
  return *this;
}

Layer &Layer::MoveRelative(Vector2D<int> pos_diff) {
  pos += pos_diff;
  return *this;
}

void Layer::DrawTo(FrameBuffer &screen) const {
  if (window) {
    window->DrawTo(screen, pos);
  }
}

void LayerManager::SetWriter(FrameBuffer *screen) { this->screen = screen; }

Layer &LayerManager::NewLayer() {
  ++latest_id;
  return *layers.emplace_back(new Layer{latest_id});
}

Layer *LayerManager::FindLayer(unsigned int id) {
  auto pred = [id](const std::unique_ptr<Layer> &elem) {
    return elem->ID() == id;
  };
  auto it = std::find_if(layers.begin(), layers.end(), pred);
  if (it == layers.end()) {
    return nullptr;
  }
  return it->get();
}

void LayerManager::Move(unsigned int id, Vector2D<int> new_position) {
  FindLayer(id)->Move(new_position);
}

void LayerManager::MoveRelative(unsigned int id, Vector2D<int> pos_diff) {
  FindLayer(id)->MoveRelative(pos_diff);
}

void LayerManager::Draw() const {
  for (auto layer : layer_stack) {
    layer->DrawTo(*screen);
  }
}

void LayerManager::Hide(unsigned int id) {
  auto layer = FindLayer(id);
  auto pos = std::find(layer_stack.begin(), layer_stack.end(), layer);
  if (pos != layer_stack.end()) {
    layer_stack.erase(pos);
  }
}

void LayerManager::UpDown(unsigned int id, int new_height) {
  if (new_height < 0) {
    Hide(id);
    return;
  }
  if (new_height > layer_stack.size()) {
    new_height = layer_stack.size();
  }

  auto layer = FindLayer(id);
  auto old_pos = std::find(layer_stack.begin(), layer_stack.end(), layer);
  auto new_pos = layer_stack.begin() + new_height;

  if (old_pos == layer_stack.end()) {
    layer_stack.insert(new_pos, layer);
    return;
  }

  if (new_pos == layer_stack.end()) {
    --new_pos;
  }
  layer_stack.erase(old_pos);
  layer_stack.insert(new_pos, layer);
}
