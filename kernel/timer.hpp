#pragma once

#include <cstdint>
#include <deque>
#include <limits>
#include <queue>

#include "message.hpp"

void InitializeLAPICTimer();
void StartLAPICTimer();
uint32_t LAPICTimerElapsed();
void StopLAPICTimer();

class Timer {
public:
  Timer(unsigned long timeout, int value);
  unsigned long Timeout() const { return timeout; }
  int Value() const { return value; }

private:
  unsigned long timeout;
  int value;
};

inline bool operator<(const Timer &lhs, const Timer &rhs) {
  return lhs.Timeout() > rhs.Timeout();
}

class TimerManager {
public:
  TimerManager();
  void AddTimer(const Timer &timer);
  bool Tick();
  unsigned long CurrentTick() const { return tick; }

private:
  volatile unsigned long tick{0};
  std::priority_queue<Timer> timers{};
};

inline TimerManager *timer_manager;
inline unsigned long lapic_timer_freq;
const int kTimerFreq = 100;

const int kTaskTimerPeriod = static_cast<int>(kTimerFreq * 0.02);
const int kTaskTimerValue = std::numeric_limits<int>::min();

void LAPICTimerOnInterrupt();
