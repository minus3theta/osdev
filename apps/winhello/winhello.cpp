#include "../syscall.hpp"

extern "C" int main(int argc, char **argv) {
  SyscallOpenWindow(200, 100, 10, 10, "winhello");
  SyscallExit(0);
}
