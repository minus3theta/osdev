#include <array>
#include <deque>
#include <memory>

#include "fat.hpp"
#include "graphics.hpp"
#include "window.hpp"

class Terminal {
public:
  static const int kRows = 15, kColumns = 60;
  static const int kLineMax = 128;

  Terminal();
  unsigned int LayerID() const { return layer_id; }
  Rectangle<int> BlinkCursor();
  Rectangle<int> InputKey(uint8_t modifier, uint8_t keycode, char ascii);
  void Print(char c);
  void Print(const char *s);
  void ExecuteLine();
  Error ExecuteFile(const fat::DirectoryEntry &file_entry, char *command,
                    char *first_arg);

private:
  std::shared_ptr<ToplevelWindow> window;
  unsigned int layer_id;

  Vector2D<int> cursor{0, 0};
  bool cursor_visible{false};
  void DrawCursor(bool visible);
  Vector2D<int> CalcCursorPos() const;

  int linebuf_index{0};
  std::array<char, kLineMax> linebuf{};
  void Scroll1();

  std::deque<std::array<char, kLineMax>> cmd_history{};
  int cmd_history_index{-1};
  Rectangle<int> HistoryUpDown(int direction);
};

void TaskTerminal(uint64_t task_id, int64_t data);
