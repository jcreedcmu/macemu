#include "debug.h"

#include <Windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

FILE *debug_log = NULL;
WindowPtr gLogWindow = NULL;

void init_debug() {
  // debug log
  debug_log = fopen("out", "w");
}

void logger(const char *format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[1024];
  size_t n = vsnprintf((char *)buffer, 1024, format, args);
  va_end(args);

  // FIXME: display to window as well
  if (n >= 0) {
    fwrite(buffer, 1, n, debug_log);
  }
}
