#include <libtwelf.h>  // twelf core

#include "debug.h"
#include "framework.h"

int main() {
  // Debugging Log
  init_debug();

  int argc = 1;
  const char *argv[] = {
      "twelf",
  };
  logger("about to twelf-open\r");
  twelf_server_open(argc, argv);
  logger("twelf-opened\r");

  // call framework
  FrameworkEntry();
}
