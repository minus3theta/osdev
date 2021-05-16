#pragma once

#include <cstdint>

void InitializeLAPICTimer();
void StartLAPICTimer();
uint32_t LAPICTimerElapsed();
void StopLAPICTimer();

class TimerManager {
public:
  void Tick();
  unsigned long CurrentTick() const { return tick; }

private:
  volatile unsigned long tick{0};
};

inline TimerManager *timer_manager;

void LAPICTimerOnInterrupt();
