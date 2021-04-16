#pragma once

enum LogLevel {
  kError = 3,
  kWarn = 4,
  kInfo = 6,
  kDebug = 7,
};

void setLogLevel(LogLevel level);

int Log(LogLevel level, const char* format, ...);
