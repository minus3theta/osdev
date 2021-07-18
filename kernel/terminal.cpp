#include "terminal.hpp"

#include <memory>

#include "graphics.hpp"
#include "layer.hpp"
#include "message.hpp"
#include "task.hpp"

Terminal::Terminal() {
  window = std::make_shared<ToplevelWindow>(
      kColumns * 8 + 8 + ToplevelWindow::kMarginX,
      kRows * 16 + 8 + ToplevelWindow::kMarginY, screen_config.pixel_format,
      "MikanTerm");
  DrawTerminal(*window->InnerWriter(), {0, 0}, window->InnerSize());

  layer_id =
      layer_manager->NewLayer().SetWindow(window).SetDraggable(true).ID();
}

Rectangle<int> Terminal::BlinkCursor() {
  cursor_visible = !cursor_visible;
  DrawCursor(cursor_visible);

  return {ToplevelWindow::kTopLeftMargin +
              Vector2D<int>{4 + 8 * cursor.x, 5 + 16 * cursor.y},
          {7, 15}};
}

void Terminal::DrawCursor(bool visible) {
  const auto color = visible ? ToColor(0xffffff) : ToColor(0);
  const auto pos = Vector2D<int>{4 + 8 * cursor.x, 5 + 16 * cursor.y};
  FillRectangle(*window->InnerWriter(), pos, {7, 15}, color);
}

void TaskTerminal(uint64_t task_id, int64_t data) {
  __asm__("cli");
  Task &task = task_manager->CurrentTask();
  Terminal *terminal = new Terminal;
  layer_manager->Move(terminal->LayerID(), {100, 200});
  active_layer->Activate(terminal->LayerID());
  __asm__("sti");

  while (true) {
    __asm__("cli");
    auto msg = task.ReceiveMessage();
    if (!msg) {
      task.Sleep();
      __asm__("sti");
      continue;
    }

    switch (msg->type) {
    case Message::kTimerTimeout: {
      const auto area = terminal->BlinkCursor();
      Message msg = MakeLayerMessage(task_id, terminal->LayerID(),
                                     LayerOperation::DrawArea, area);
      __asm__("cli");
      task_manager->SendMessage(1, msg);
      __asm__("sti");
    } break;
    default:
      break;
    }
  }
}
