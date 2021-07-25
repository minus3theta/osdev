#include "mouse.hpp"

#include <limits>
#include <memory>

#include "graphics.hpp"
#include "layer.hpp"
#include "task.hpp"
#include "usb/classdriver/mouse.hpp"

namespace {
const char mouse_cursor_shape[kMouseCursorHeight][kMouseCursorWidth + 1] = {
    // clang-format off
  "@              ",
  "@@             ",
  "@.@            ",
  "@..@           ",
  "@...@          ",
  "@....@         ",
  "@.....@        ",
  "@......@       ",
  "@.......@      ",
  "@........@     ",
  "@.........@    ",
  "@..........@   ",
  "@...........@  ",
  "@............@ ",
  "@......@@@@@@@@",
  "@......@       ",
  "@....@@.@      ",
  "@...@ @.@      ",
  "@..@   @.@     ",
  "@.@    @.@     ",
  "@@      @.@    ",
  "@       @.@    ",
  "         @.@   ",
  "         @@@   ",
    // clang-format on
};

void SendMouseMessage(Vector2D<int> newpos, Vector2D<int> posdiff,
                      uint8_t buttons) {
  const auto act = active_layer->GetActive();
  if (!act) {
    return;
  }
  const auto layer = layer_manager->FindLayer(act);

  const auto task_it = layer_task_map->find(act);
  if (task_it == layer_task_map->end()) {
    return;
  }

  if (posdiff.x != 0 || posdiff.y != 0) {
    const auto relpos = newpos - layer->GetPosition();
    Message msg{Message::kMouseMove};
    msg.arg.mouse_move.x = relpos.x;
    msg.arg.mouse_move.y = relpos.y;
    msg.arg.mouse_move.dx = posdiff.x;
    msg.arg.mouse_move.dy = posdiff.y;
    msg.arg.mouse_move.buttons = buttons;
    task_manager->SendMessage(task_it->second, msg);
  }
}
} // namespace

void DrawMouseCursor(PixelWriter *pixel_writer, Vector2D<int> position) {
  for (int dy = 0; dy < kMouseCursorHeight; ++dy) {
    for (int dx = 0; dx < kMouseCursorWidth; ++dx) {
      const auto p = Vector2D<int>{dx, dy};
      if (mouse_cursor_shape[dy][dx] == '@') {
        pixel_writer->Write(position + p, {0, 0, 0});
      } else if (mouse_cursor_shape[dy][dx] == '.') {
        pixel_writer->Write(position + p, {255, 255, 255});
      } else {
        pixel_writer->Write(position + p, kMouseTransparentColor);
      }
    }
  }
}

Mouse::Mouse(unsigned int layer_id) : layer_id(layer_id) {}

void Mouse::SetPosition(Vector2D<int> position) {
  this->position = position;
  layer_manager->Move(layer_id, this->position);
}

void Mouse::OnInterrupt(uint8_t buttons, int8_t displacement_x,
                        int8_t displacement_y) {
  const auto oldpos = position;
  auto newpos = position + Vector2D<int>{displacement_x, displacement_y};
  newpos = ElementMin(newpos, ScreenSize() + Vector2D<int>{-1, -1});
  position = ElementMax(newpos, {0, 0});

  const auto posdiff = position - oldpos;

  layer_manager->Move(layer_id, position);

  const bool previous_left_pressed = previous_buttons & 0x01;
  const bool left_pressed = buttons & 0x01;
  if (!previous_left_pressed && left_pressed) {
    auto layer = layer_manager->FindLayerByPosition(position, layer_id);
    if (layer && layer->IsDraggable()) {
      drag_layer_id = layer->ID();
      active_layer->Activate(layer->ID());
    } else {
      active_layer->Activate(0);
    }
  } else if (previous_left_pressed && left_pressed) {
    if (drag_layer_id > 0) {
      layer_manager->MoveRelative(drag_layer_id, posdiff);
    }
  } else if (previous_left_pressed && !left_pressed) {
    drag_layer_id = 0;
  }

  if (drag_layer_id == 0) {
    SendMouseMessage(newpos, posdiff, buttons);
  }

  previous_buttons = buttons;
}

void InitializeMouse() {
  auto mouse_window = std::make_shared<Window>(
      kMouseCursorWidth, kMouseCursorHeight, screen_config.pixel_format);
  mouse_window->SetTransparentColor(kMouseTransparentColor);
  DrawMouseCursor(mouse_window.get(), {0, 0});

  auto mouse_layer_id = layer_manager->NewLayer().SetWindow(mouse_window).ID();

  auto mouse = std::make_shared<Mouse>(mouse_layer_id);
  mouse->SetPosition({200, 200});
  layer_manager->UpDown(mouse->LayerID(), std::numeric_limits<int>::max());

  usb::HIDMouseDriver::default_observer =
      [mouse](uint8_t buttons, int8_t displacement_x, int8_t displacement_y) {
        mouse->OnInterrupt(buttons, displacement_x, displacement_y);
      };

  active_layer->SetMouseLayer(mouse_layer_id);
}
