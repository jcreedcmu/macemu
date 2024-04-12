#include <libtwelf.h>  // twelf core

#include "debug.h"
#include "framework.h"

int main() {
  // Debugging Log
  init_debug();

  const char *argv[] = {
      "twelf",
      "@MLton", "gc-messages", "--",
  };
  logger("about to twelf-open");
  twelf_server_open(sizeof(argv) / sizeof(argv[0]), argv);
  logger("twelf-opened");

  // call framework
  FrameworkEntry();
}
