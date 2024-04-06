#include <libtwelf.h>  // twelf core
#include <stdio.h>

#include "framework.h"

int main() {
  // Debugging Log
  stdout = stderr = fopen("out", "w");
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  int argc = 1;
  const char *argv[] = {
      "twelf",
  };
  printf("about to twelf-open\r");
  twelf_server_open(argc, argv);
  printf("twelf-opened\r");

  // call framework
  FrameworkEntry();
}
