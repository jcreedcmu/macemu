#include <sys/types.h>

/***
 * We need to define *both* _consolewrite and _consoleread to somehow
 * override the ones in default libRetroConsole.a, I don't understand
 * why yet.
 */

extern ssize_t _consolewrite(int fd, const void *buf, size_t count) {
    return count;
}

extern ssize_t _consoleread(int fd, void *buf, size_t count) {
    return count;
}
