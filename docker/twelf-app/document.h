#pragma once

#include "api.h"

typedef enum { TwelfWinDocument, TwelfWinAbout } TwelfWinType;
typedef enum { TwelfDocument, TwelfOutput, TwelfLog } DocType;

// All windows in this application should be of this type:
typedef struct {
  WindowRecord docWindow;
  TwelfWinType winType;
} TwelfWinRecord, *TwelfWinPtr;

// If winType == TwelfWinDocument, then assume all these fields exist:
typedef struct {
  WindowRecord window;
  TwelfWinType winType;
  DocType docType;
  TEHandle docTE;
  ControlHandle docVScroll;
  ControlHandle docHScroll;
  TEClickLoopUPP docClick;
  Boolean fsSpecSet;
  FSSpec fsSpec;
  Boolean dirty;
} DocumentRecord, *DocumentPeek, *DocumentPtr;

// If winType == TwelfWinAbout, then assume all these fields exist:
typedef struct {
  WindowRecord window;
  TwelfWinType winType;
  TEHandle te;
  PicHandle pic;
  PicHandle bwPic;
  Rect picRect;
} AboutRecord, *AboutPtr;

#define getDoc(window) ((DocumentPeek)(window))   // FIXME(safety): check tag
#define getAboutDoc(window) ((AboutPtr)(window))  // FIXME(safety): check tag

Boolean isReadOnly(DocType docType);
