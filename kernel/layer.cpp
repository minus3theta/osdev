#include "layer.hpp"

#include <algorithm>
#include <cstdlib>
#include <memory>

#include "console.hpp"
#include "frame_buffer.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "logger.hpp"
#include "message.hpp"

Layer::Layer(unsigned int id) : id(id) {}

Layer &Layer::SetWindow(const std::shared_ptr<Window> &window) {
  this->window = window;
  return *this;
}

std::shared_ptr<Window> Layer::GetWindow() const {
  return window;
}

unsigned int Layer::ID() const {
  return id;
}

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

Vector2D<int> Layer::GetPosition() const {
  return pos;
}

bool Layer::IsDraggable() const {
  return draggable;
}

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

int LayerManager::GetHeight(unsigned int id) {
  for (int h = 0; h < layer_stack.size(); ++h) {
    if (layer_stack[h]->ID() == id) {
      return h;
    }
  }
  return -1;
}

namespace {
FrameBuffer *screen;
}

void InitializeLayer() {
  const auto screen_size = ScreenSize();

  auto bgwindow = std::make_shared<Window>(screen_size.x, screen_size.y,
                                           screen_config.pixel_format);

  DrawDesktop(*bgwindow);

  auto console_window = std::make_shared<Window>(
      Console::kColumns * 8, Console::kRows * 16, screen_config.pixel_format);
  console->SetWindow(console_window);

  if (auto err = screen->Initialize(screen_config)) {
    Log(kError, "failed to initialize frame buffer: %s at %s:%d\n", err.Name(),
        err.File(), err.Line());
    exit(1);
  }

  layer_manager = new LayerManager;
  layer_manager->SetWriter(screen);

  auto bglayer_id =
      layer_manager->NewLayer().SetWindow(bgwindow).Move({0, 0}).ID();
  console->SetLayerID(
      layer_manager->NewLayer().SetWindow(console_window).Move({0, 0}).ID());

  layer_manager->UpDown(bglayer_id, 0);
  layer_manager->UpDown(console->LayerID(), 1);

  active_layer = new ActiveLayer{*layer_manager};
}

void ProcessLayerMessage(const Message &msg) {
  const auto &arg = msg.arg.layer;
  switch (arg.op) {
  case LayerOperation::Move:
    layer_manager->Move(arg.layer_id, {arg.x, arg.y});
    break;
  case LayerOperation::MoveRelative:
    layer_manager->MoveRelative(arg.layer_id, {arg.x, arg.y});
    break;
  case LayerOperation::Draw:
    layer_manager->Draw(arg.layer_id);
    break;
  }
}

ActiveLayer::ActiveLayer(LayerManager &manager) : manager{manager} {}

void ActiveLayer::SetMouseLayer(unsigned int mouse_layer) {
  this->mouse_layer = mouse_layer;
}

void ActiveLayer::Activate(unsigned int layer_id) {
  if (active_layer == layer_id) {
    return;
  }

  if (active_layer > 0) {
    Layer *layer = manager.FindLayer(active_layer);
    layer->GetWindow()->Deactivate();
    manager.Draw(active_layer);
  }

  active_layer = layer_id;
  if (active_layer > 0) {
    Layer *layer = manager.FindLayer(active_layer);
    layer->GetWindow()->Activate();
    manager.UpDown(active_layer, manager.GetHeight(mouse_layer) - 1);
    manager.Draw(active_layer);
  }
}
