#include <cstdlib>

#include "../syscall.hpp"

char table[3 * 1024 * 1024];

extern "C" int main(int argc, char **argv) {
  SyscallExit(atoi(argv[1]));
}
