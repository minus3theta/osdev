#include "terminal.hpp"

#include <cstdio>
#include <cstring>
#include <memory>
#include <utility>

#include "fat.hpp"
#include "font.hpp"
#include "graphics.hpp"
#include "layer.hpp"
#include "logger.hpp"
#include "message.hpp"
#include "pci.hpp"
#include "task.hpp"
#include "window.hpp"

Terminal::Terminal() {
  window = std::make_shared<ToplevelWindow>(
      kColumns * 8 + 8 + ToplevelWindow::kMarginX,
      kRows * 16 + 8 + ToplevelWindow::kMarginY, screen_config.pixel_format,
      "MikanTerm");
  DrawTerminal(*window->InnerWriter(), {0, 0}, window->InnerSize());

  layer_id =
      layer_manager->NewLayer().SetWindow(window).SetDraggable(true).ID();

  Print(">");
  cmd_history.resize(8);
}

Rectangle<int> Terminal::BlinkCursor() {
  cursor_visible = !cursor_visible;
  DrawCursor(cursor_visible);

  return {CalcCursorPos(), {7, 15}};
}

void Terminal::DrawCursor(bool visible) {
  const auto color = visible ? ToColor(0xffffff) : ToColor(0);
  FillRectangle(*window, CalcCursorPos(), {7, 15}, color);
}

Vector2D<int> Terminal::CalcCursorPos() const {
  return ToplevelWindow::kTopLeftMargin +
         Vector2D<int>{4 + 8 * cursor.x, 4 + 16 * cursor.y};
}

Rectangle<int> Terminal::InputKey(uint8_t modifier, uint8_t keycode,
                                  char ascii) {
  DrawCursor(false);

  Rectangle<int> draw_area{CalcCursorPos(), {8 * 2, 16}};

  if (ascii == '\n') {
    linebuf[linebuf_index] = 0;
    if (linebuf_index > 0) {
      cmd_history.pop_back();
      cmd_history.push_front(linebuf);
    }
    linebuf_index = 0;
    cmd_history_index = -1;
    cursor.x = 0;
    Log(kWarn, "line: %s\n", &linebuf[0]);
    if (cursor.y < kRows - 1) {
      ++cursor.y;
    } else {
      Scroll1();
    }
    ExecuteLine();
    Print(">");
    draw_area.pos = ToplevelWindow::kTopLeftMargin;
    draw_area.size = window->InnerSize();
  } else if (ascii == '\b') {
    if (cursor.x > 0) {
      --cursor.x;
      FillRectangle(*window, CalcCursorPos(), {8, 16}, ToColor(0));
      draw_area.pos = CalcCursorPos();

      if (linebuf_index > 0) {
        --linebuf_index;
      }
    }
  } else if (ascii != 0) {
    if (cursor.x < kColumns - 1 && linebuf_index < kLineMax - 1) {
      linebuf[linebuf_index] = ascii;
      ++linebuf_index;
      WriteAscii(*window, CalcCursorPos(), ascii, ToColor(0xffffff));
      ++cursor.x;
    }
  } else if (keycode == 0x51) { // down arrow
    draw_area = HistoryUpDown(-1);
  } else if (keycode == 0x52) { // up arrow
    draw_area = HistoryUpDown(1);
  }

  DrawCursor(true);

  return draw_area;
}

void Terminal::Scroll1() {
  Rectangle<int> move_src{ToplevelWindow::kTopLeftMargin +
                              Vector2D<int>{4, 4 + 16},
                          {8 * kColumns, 16 * (kRows - 1)}};
  window->Move(ToplevelWindow::kTopLeftMargin + Vector2D<int>{4, 4}, move_src);
  FillRectangle(*window->InnerWriter(), {4, 4 + 16 * cursor.y},
                {8 * kColumns, 16}, ToColor(0));
}

void Terminal::Print(char c) {
  auto newline = [this]() {
    cursor.x = 0;
    if (cursor.y < kRows - 1) {
      ++cursor.y;
    } else {
      Scroll1();
    }
  };

  if (c == '\n') {
    newline();
  } else {
    WriteAscii(*window, CalcCursorPos(), c, ToColor(0xffffff));
    if (cursor.x == kColumns - 1) {
      newline();
    } else {
      ++cursor.x;
    }
  }
}

void Terminal::Print(const char *s) {
  DrawCursor(false);

  while (*s) {
    Print(*s);
    ++s;
  }

  DrawCursor(true);
}

void Terminal::ExecuteLine() {
  char *command = &linebuf[0];
  char *first_arg = strchr(&linebuf[0], ' ');
  if (first_arg) {
    *first_arg = 0;
    ++first_arg;
  }

  if (strcmp(command, "echo") == 0) {
    if (first_arg) {
      Print(first_arg);
    }
    Print("\n");
  } else if (strcmp(command, "clear") == 0) {
    FillRectangle(*window->InnerWriter(), {4, 4}, {8 * kColumns, 16 * kRows},
                  ToColor(0));
    cursor.y = 0;
  } else if (strcmp(command, "lspci") == 0) {
    char s[64];
    for (int i = 0; i < pci::num_device; ++i) {
      const auto &dev = pci::devices[i];
      auto vendor_id = pci::ReadVendorId(dev.bus, dev.device, dev.function);
      sprintf(s, "%02x:%02x.%d vend=%04x head=%02x class=%02x.%02x.%02x\n",
              dev.bus, dev.device, dev.function, vendor_id, dev.header_type,
              dev.class_code.base, dev.class_code.sub,
              dev.class_code.interface);
      Print(s);
    }
  } else if (strcmp(command, "cat") == 0) {
    char s[64];

    auto file_entry = fat::FindFile(first_arg);
    if (!file_entry) {
      sprintf(s, "no such file: %s\n", first_arg);
      Print(s);
    } else {
      auto cluster = file_entry->FirstCluster();
      auto remain_bytes = file_entry->file_size;

      DrawCursor(false);
      while (cluster != 0 && cluster != fat::kEndOfClusterchain) {
        char *p = fat::GetSectorByCluster<char>(cluster);

        int i = 0;
        for (; i < fat::bytes_per_cluster && i < remain_bytes; ++i) {
          Print(*p);
          ++p;
        }
        remain_bytes -= i;
        cluster = fat::NextCluster(cluster);
      }
      DrawCursor(true);
    }
  } else if (strcmp(command, "ls") == 0) {
    auto root_dir_entries = fat::GetSectorByCluster<fat::DirectoryEntry>(
        fat::boot_volume_image->root_cluster);
    auto entries_per_cluster = fat::boot_volume_image->bytes_per_sector /
                               sizeof(fat::DirectoryEntry) *
                               fat::boot_volume_image->sectors_per_cluster;
    char base[9], ext[4];
    char s[64];
    for (int i = 0; i < entries_per_cluster; ++i) {
      fat::ReadName(root_dir_entries[i], base, ext);
      if (base[0] == 0x00) {
        break;
      } else if (static_cast<uint8_t>(base[0]) == 0xe5) {
        continue;
      } else if (root_dir_entries[i].attr == fat::Attribute::kLongName) {
        continue;
      }

      if (ext[0]) {
        sprintf(s, "%s.%s\n", base, ext);
      } else {
        sprintf(s, "%s\n", base);
      }
      Print(s);
    }
  } else if (command[0] != 0) {
    Print("no such command: ");
    Print(command);
    Print("\n");
  }
}

Rectangle<int> Terminal::HistoryUpDown(int direction) {
  if (direction == -1 && cmd_history_index >= 0) {
    --cmd_history_index;
  } else if (direction == 1 && cmd_history_index + 1 < cmd_history.size()) {
    ++cmd_history_index;
  }

  cursor.x = 1;

  const auto first_pos = CalcCursorPos();

  Rectangle<int> draw_area{first_pos, {8 * (kColumns - 1), 16}};
  FillRectangle(*window, draw_area.pos, draw_area.size, ToColor(0));

  const char *history = "";
  if (cmd_history_index >= 0) {
    history = &cmd_history[cmd_history_index][0];
  }

  strcpy(&linebuf[0], history);
  linebuf_index = strlen(history);

  WriteString(*window, first_pos, history, ToColor(0xffffff));
  cursor.x = linebuf_index + 1;
  return draw_area;
}

void TaskTerminal(uint64_t task_id, int64_t data) {
  __asm__("cli");
  Task &task = task_manager->CurrentTask();
  Terminal *terminal = new Terminal;
  layer_manager->Move(terminal->LayerID(), {100, 200});
  active_layer->Activate(terminal->LayerID());
  layer_task_map->insert(std::make_pair(terminal->LayerID(), task_id));
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
      break;
    }
    case Message::kKeyPush: {
      const auto area = terminal->InputKey(msg->arg.keyboard.modifier,
                                           msg->arg.keyboard.keycode,
                                           msg->arg.keyboard.ascii);
      Message msg = MakeLayerMessage(task_id, terminal->LayerID(),
                                     LayerOperation::DrawArea, area);
      __asm__("cli");
      task_manager->SendMessage(1, msg);
      __asm__("sti");
      break;
    }
    default:
      break;
    }
  }
}
