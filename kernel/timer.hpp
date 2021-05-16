#pragma once

#include <cstdint>
#include <deque>
#include <queue>

#include "message.hpp"

void InitializeLAPICTimer(std::deque<Message> &msg_queue);
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
  TimerManager(std::deque<Message> &msg_queue);
  void AddTimer(const Timer &timer);
  void Tick();
  unsigned long CurrentTick() const { return tick; }

private:
  volatile unsigned long tick{0};
  std::priority_queue<Timer> timers{};
  std::deque<Message> &msg_queue;
};

inline TimerManager *timer_manager;

void LAPICTimerOnInterrupt();
