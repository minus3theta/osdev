#include "layer.hpp"
#include "frame_buffer.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
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

Layer &Layer::SetDraggable(bool draggable) {
  this->draggable = draggable;
  return *this;
}

void Layer::DrawTo(FrameBuffer &screen, const Rectangle<int> &area) const {
  if (window) {
    window->DrawTo(screen, pos, area);
  }
}

Vector2D<int> Layer::GetPosition() const { return pos; }

bool Layer::IsDraggable() const { return draggable; }

void LayerManager::SetWriter(FrameBuffer *screen) {
  this->screen = screen;

  FrameBufferConfig back_config = screen->Config();
  back_config.frame_buffer = nullptr;
  back_buffer.Initialize(back_config);
}

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

void LayerManager::Move(unsigned int id, Vector2D<int> new_pos) {
  auto layer = FindLayer(id);
  const auto window_size = layer->GetWindow()->Size();
  const auto old_pos = layer->GetPosition();
  layer->Move(new_pos);
  Draw({old_pos, window_size});
  Draw(id);
}

void LayerManager::MoveRelative(unsigned int id, Vector2D<int> pos_diff) {
  auto layer = FindLayer(id);
  const auto window_size = layer->GetWindow()->Size();
  const auto old_pos = layer->GetPosition();
  layer->MoveRelative(pos_diff);
  Draw({old_pos, window_size});
  Draw(id);
}

void LayerManager::Draw(const Rectangle<int> &area) const {
  for (auto layer : layer_stack) {
    layer->DrawTo(back_buffer, area);
  }
  screen->Copy(area.pos, back_buffer, area);
}

void LayerManager::Draw(unsigned int id) const {
  bool draw = false;
  Rectangle<int> window_area;
  for (auto layer : layer_stack) {
    if (layer->ID() == id) {
      window_area.size = layer->GetWindow()->Size();
      window_area.pos = layer->GetPosition();
      draw = true;
    }
    if (draw) {
      layer->DrawTo(back_buffer, window_area);
    }
  }
  screen->Copy(window_area.pos, back_buffer, window_area);
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

Layer *LayerManager::FindLayerByPosition(Vector2D<int> pos,
                                         unsigned int exclude_id) const {
  auto pred = [pos, exclude_id](Layer *layer) {
    if (layer->ID() == exclude_id) {
      return false;
    }
    const auto &win = layer->GetWindow();
    if (!win) {
      return false;
    }
    const auto win_pos = layer->GetPosition();
    const auto win_end_pos = win_pos + win->Size();
    return win_pos.x <= pos.x && pos.x < win_end_pos.x && win_pos.y <= pos.y &&
           pos.y < win_end_pos.y;
  };
  auto it = std::find_if(layer_stack.rbegin(), layer_stack.rend(), pred);
  if (it == layer_stack.rend()) {
    return nullptr;
  }
  return *it;
}