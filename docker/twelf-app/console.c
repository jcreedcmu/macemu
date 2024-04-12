#include "console.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "document.h"
#include "global-state.h"

/***
 * We need to define *both* _consolewrite and _consoleread to somehow
 * override the ones in default libRetroConsole.a, I don't understand
 * why yet.
 */

TEHandle stdoutDest = NULL;

void setOutputDest(TEHandle output) { stdoutDest = output; }

TEHandle getOutputDest() { return stdoutDest; }

extern ssize_t _consolewrite(int fd, const void *buf, size_t count) {
  // stdoutDest is where our caller would prefer stdout to appear, if
  // it is non-NULL. The reason we let the caller decide where stdout
  // is redirected is keeping the possibility open of having several
  // output windows, one for each input window.
  TEHandle outputDest = stdoutDest;

  // Treat stderr specially. Since we might have debugging log
  // messages arising from circumstances that are not really
  // "per-input-file", we just lump them all together into the global
  // debug log window. In any event, stderr should never go to the
  // output window.
  if (fd == 2) {
    if (gLogWindow != NULL) {
      outputDest = getDoc(gLogWindow)->docTE;
    } else {
      outputDest = NULL;
    }
  }

  if (outputDest != NULL) {
    char *tmpBuf = malloc(count);
    strncpy(tmpBuf, (char *)buf, count);
    for (size_t i = 0; i < count; i++) {
      // Convert from unix to macos conventional EOL-character
      if (tmpBuf[i] == '\n') {
        tmpBuf[i] = '\r';
      }
    }
    TEInsert(tmpBuf, count, outputDest);
    free(tmpBuf);
    return count;
  } else {
    // send to /dev/null
    return count;
  }
}

extern ssize_t _consoleread(int fd, void *buf, size_t count) { return count; }
