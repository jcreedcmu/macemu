#include "multiversal-stubs.h"

// The purpose of this file is to expediently fill in functions that
// were missing in
// https://github.com/autc04/multiversal
// but they can be deleted if they wind up being upstreamed there.

// Adapted from
// https://github.com/autc04/executor/blob/27c8ef28bc0ea29e7621466068c4e30aea664562/src/textedit/teScrap.cpp#L17-L30
pascal OSErr TEFromScrap() {
  long unused_offset;
  long len;
  Handle sh = LMGetTEScrpHandle();
  len = GetScrap(sh, 'TEXT', &unused_offset);
  if (len < 0) {
    EmptyHandle(sh);
    LMSetTEScrpLength(0);
    return len;
  } else {
    LMSetTEScrpLength(len);
    return noErr;
  }
}

pascal OSErr TEToScrap() {
  PScrapStuff scrapInfo = InfoScrap();
  // XXX how detect whether scrap hasn't been initialized?
  // Is checking for length = 0 enough?
  // should return noScrapErr = -100 in that case
  long scrapLength = LMGetTEScrpLength();
  if (scrapLength == 0) {
    return noScrapErr;
  }
  Handle h = LMGetTEScrpHandle();
  return PutScrap(scrapLength, 'TEXT', *h);
}

pascal long TEGetScrapLength() { return LMGetTEScrpLength(); }
