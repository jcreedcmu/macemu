#include <libtwelf.h>  // twelf core

#include "debug.h"
#include "framework.h"

void InitTwelf(void) {
  const char *argv[] = {
      "twelf",
      "@MLton",
      "gc-messages",      // Print garbage collection messages.
      "ram-slop", "0.9",  // Use 90% of Application Heap Limit.
      "--",
  };
  logger("about to twelf-open");
  twelf_server_open(sizeof(argv) / sizeof(argv[0]), argv);
  logger("twelf-opened");
}

int main(void) {
  // Debugging Log
  init_debug();

  // call framework
  FrameworkEntry();
}
