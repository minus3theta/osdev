#include "timer.hpp"

#include <cstdint>
#include <deque>
#include <limits>

#include "acpi.hpp"
#include "interrupt.hpp"
#include "message.hpp"
#include "task.hpp"

namespace {
const uint32_t kCountMax = 0xffffffffu;
volatile uint32_t &lvt_timer = *reinterpret_cast<uint32_t *>(0xfee00320);
volatile uint32_t &initial_count = *reinterpret_cast<uint32_t *>(0xfee00380);
volatile uint32_t &current_count = *reinterpret_cast<uint32_t *>(0xfee00390);
volatile uint32_t &divide_config = *reinterpret_cast<uint32_t *>(0xfee003e0);
} // namespace

void InitializeLAPICTimer() {
  timer_manager = new TimerManager;

  divide_config = 0b1011;
  lvt_timer = (0b001 << 16);

  StartLAPICTimer();
  acpi::WaitMilliseconds(100);
  const auto elapsed = LAPICTimerElapsed();
  StopLAPICTimer();

  lapic_timer_freq = static_cast<unsigned long>(elapsed) * 10;

  divide_config = 0b1011;
  lvt_timer = (0b010 << 16) | InterruptVector::kLAPICTimer;
  initial_count = lapic_timer_freq / kTimerFreq;
}

void StartLAPICTimer() { initial_count = kCountMax; }

uint32_t LAPICTimerElapsed() { return kCountMax - current_count; }

void StopLAPICTimer() { initial_count = 0; }

Timer::Timer(unsigned long timeout, int value)
    : timeout(timeout), value(value) {}

TimerManager::TimerManager() {
  timers.push(Timer{std::numeric_limits<unsigned long>::max(), -1});
}

void TimerManager::AddTimer(const Timer &timer) { timers.push(timer); }

bool TimerManager::Tick() {
  ++tick;

  bool task_timer_timeout = false;
  while (true) {
    const auto &t = timers.top();
    if (t.Timeout() > tick) {
      break;
    }

    if (t.Value() == kTaskTimerValue) {
      task_timer_timeout = true;
      timers.pop();
      timers.push(Timer{tick + kTaskTimerPeriod, kTaskTimerValue});
      continue;
    }

    Message m{Message::kTimerTimeout};
    m.arg.timer.timeout = t.Timeout();
    m.arg.timer.value = t.Value();
    task_manager->SendMessage(1, m);

    timers.pop();
  }

  return task_timer_timeout;
}

void LAPICTimerOnInterrupt() {
  const bool task_timer_timeout = timer_manager->Tick();
  NotifyEndOfInterrupt();

  if (task_timer_timeout) {
    task_manager->SwitchTask();
  }
}
