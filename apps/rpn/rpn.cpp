#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "../../kernel/logger.hpp"

int stack_ptr;
long stack[100];

long Pop() {
  long value = stack[stack_ptr--];
  return value;
}

void Push(long value) {
  stack[++stack_ptr] = value;
}

extern "C" int64_t SyscallLogString(LogLevel, const char *);

extern "C" int64_t SyscallExit(int);

extern "C" int main(int argc, char **argv) {
  stack_ptr = -1;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "+") == 0) {
      long b = Pop();
      long a = Pop();
      Push(a + b);
      SyscallLogString(kWarn, "+");
    } else if (strcmp(argv[i], "-") == 0) {
      long b = Pop();
      long a = Pop();
      Push(a - b);
      SyscallLogString(kWarn, "-");
    } else {
      long a = atol(argv[i]);
      Push(a);
      SyscallLogString(kWarn, "#");
    }
  }
  long result = 0;
  if (stack_ptr >= 0) {
    result = Pop();
  }
  printf("%ld\n", result);
  SyscallExit(static_cast<int>(result));
}
