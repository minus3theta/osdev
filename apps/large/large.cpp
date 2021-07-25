#include <cstdlib>

#include "../syscall.h"

char table[3 * 1024 * 1024];

extern "C" int main(int argc, char **argv) {
  exit(atoi(argv[1]));
}
