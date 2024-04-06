#include "console.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/***
 * We need to define *both* _consolewrite and _consoleread to somehow
 * override the ones in default libRetroConsole.a, I don't understand
 * why yet.
 */

TEHandle outputDest = NULL;

void setOutputDest(TEHandle output) { outputDest = output; }

TEHandle getOutputDest() { return outputDest; }

extern ssize_t _consolewrite(int fd, const void *buf, size_t count) {
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
