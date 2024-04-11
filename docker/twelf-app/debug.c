#include "debug.h"

#include <TextEdit.h>
#include <Windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "document.h"
#include "global-state.h"
#include "scrolling.h"
#include "windows.h"

FILE *debug_log = NULL;

void init_debug() {
  // debug log
  debug_log = fopen("out", "w");
}

void logger(const char *format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[1024];
  size_t n = vsnprintf((char *)buffer, 1023, format, args);
  va_end(args);

  if (n >= 0) {
    fwrite(buffer, 1, n, debug_log);
    fwrite("\r", 1, 1, debug_log);
    if (gLogWindow) {
      DocumentPtr doc = getDoc(gLogWindow);
      TEHandle te = doc->docTE;
      // FIXME: should always go to end
      TEInsert(buffer, n, te);
      TEInsert("\r", 1, te);
      TESelView(te);
      AdjustScrollValues(doc, false);
      DrawWindow(gLogWindow);
    }
  } else {
    char *error = "couldn't write to debug log?\r";
    fwrite(error, 1, strlen(error), debug_log);
  }
  fflush(debug_log);
}
