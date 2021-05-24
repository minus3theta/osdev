#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <limits>
#include <memory>

#include "acpi.hpp"
#include "asmfunc.hpp"
#include "console.hpp"
#include "error.hpp"
#include "font.hpp"
#include "frame_buffer.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "interrupt.hpp"
#include "layer.hpp"
#include "logger.hpp"
#include "memory_manager.hpp"
#include "memory_map.hpp"
#include "message.hpp"
#include "mouse.hpp"
#include "paging.hpp"
#include "pci.hpp"
#include "segment.hpp"
#include "timer.hpp"
#include "usb/device.hpp"
#include "usb/memory.hpp"
#include "usb/xhci/trb.hpp"
#include "usb/xhci/xhci.hpp"
#include "window.hpp"

int printk(const char *format, ...) {
  va_list ap;
  int result;
  char s[1024];

  va_start(ap, format);
  result = vsprintf(s, format, ap);
  va_end(ap);

  console->PutString(s);
  return result;
}

std::shared_ptr<Window> main_window;
unsigned int main_window_layer_id;
void InitializeMainWindow() {
  main_window = std::make_shared<Window>(160, 68, screen_config.pixel_format);
  DrawWindow(*main_window, "Hello Window");

  main_window_layer_id = layer_manager->NewLayer()
                             .SetWindow(main_window)
                             .SetDraggable(true)
                             .Move({300, 100})
                             .ID();

  layer_manager->UpDown(main_window_layer_id, std::numeric_limits<int>::max());
}

std::deque<Message> *main_queue;

alignas(16) uint8_t kernel_main_stack[1024 * 1024];

extern "C" void
KernelMainNewStack(const FrameBufferConfig &frame_buffer_config_ref,
                   const MemoryMap &memory_map_ref,
                   const acpi::RSDP &acpi_table) {
  MemoryMap memory_map(memory_map_ref);

  InitializeGraphics(frame_buffer_config_ref);
  InitializeConsole();

  printk("Welcome to MikanOS!\n");
  setLogLevel(kWarn);

  InitializeSegmentation();
  InitializePaging();
  InitializeMemoryManager(memory_map);
  ::main_queue = new std::deque<Message>(32);
  InitializeInterrupt(main_queue);

  InitializePCI();
  usb::xhci::Initialize();

  InitializeLayer();
  InitializeMainWindow();
  InitializeMouse();
  layer_manager->Draw({{0, 0}, ScreenSize()});

  acpi::Initialize(acpi_table);
  InitializeLAPICTimer(*main_queue);

  char str[128];

  while (true) {
    __asm__("cli");
    const auto tick = timer_manager->CurrentTick();
    __asm__("sti");

    sprintf(str, "%010lu", tick);
    FillRectangle(*main_window, {24, 28}, {8 * 10, 16}, {0xc6, 0xc6, 0xc6});
    WriteString(*main_window, {24, 28}, str, {0, 0, 0});
    layer_manager->Draw(main_window_layer_id);

    __asm__("cli");
    if (main_queue->size() == 0) {
      __asm__("sti\n\thlt");
      continue;
    }

    Message msg = main_queue->front();
    main_queue->pop_front();
    __asm__("sti");

    switch (msg.type) {
    case Message::kInterruptXHCI:
      usb::xhci::ProcessEvents();
      break;
    case Message::kTimerTimeout:
      break;
    default:
      Log(kError, "Unknown message type: %d\n", static_cast<int>(msg.type));
    }
  }

  while (1) {
    __asm__("hlt");
  }
}

extern "C" void __cxa_pure_virtual() {
  while (1) {
    __asm__("hlt");
  }
}
