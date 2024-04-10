#include "windows.h"

#include <string.h>

#include "about.h"
#include "api.h"
#include "asmclikloop.h"
#include "consts.h"
#include "dialogs.h"
#include "document.h"
#include "global-state.h"
#include "resource-consts.h"
#include "scrolling.h"
#include "view-rects.h"

/* Create a new document and window. Returns NULL on failure, pointer to new
 * document on success. */

WindowPtr getOutputWindow() {
  if (!gOutputWindow) {
    gOutputWindow = mkDocumentWindow(TwelfOutput);
  }
  return gOutputWindow;
}

// FIXME(safety): should return DocumentPtr
WindowPtr mkDocumentWindow(DocType docType) {
  Boolean good;
  Ptr storage;
  Rect destRect, viewRect, windowRect;
  DocumentPeek doc;

  int offset = gNumDocuments * 16;
  SetRect(&windowRect, 3, 40, 403, 340);
  OffsetRect(&windowRect, offset, offset);

  storage = NewPtr(sizeof(DocumentRecord));
  if (storage != nil) {
    WindowPtr window;

    switch (docType) {
      case TwelfDocument:
        window = NewWindow(storage, &windowRect, "\puntitled", true,
                           documentProc, (WindowPtr)-1, true, 0);
        break;
      case TwelfOutput:
        window = NewWindow(storage, &windowRect, "\pTwelf Output", true,
                           documentProc, (WindowPtr)-1, true, 0);
        break;
    }

    if (window != nil) {
      /* this will be decremented when we call DoCloseWindow */
      gNumDocuments += 1;
      good = false;
      SetPort(window);
      doc = (DocumentPeek)window;
      GetTERect(window, &viewRect);
      destRect = viewRect;
      destRect.right = destRect.left + kMaxDocWidth;

      // Get font metrics
      int fontFamily = kFontIDMonaco;
      int fontSize = 9;
      TextFont(fontFamily);
      TextSize(fontSize);
      FontInfo finfo;
      GetFontInfo(&finfo);

      TEHandle te = TENew(&destRect, &viewRect);
      doc->docTE = te;

      (*te)->txFont = kFontIDMonaco;
      (*te)->txSize = fontSize;
      (*te)->fontAscent = finfo.ascent;
      (*te)->lineHeight = finfo.ascent + finfo.descent + 1;

      doc->fsSpecSet = false;
      doc->dirty = false;
      doc->docType = docType;
      doc->winType = TwelfWinDocument;

      good =
          doc->docTE != nil; /* if TENew succeeded, we have a good document */
      if (good) {            /* 1.02 - good document? -- proceed */
        AdjustViewRect(doc->docTE);
        TEAutoView(true, doc->docTE);
        doc->docClick = (*doc->docTE)->clikLoop;
        (*doc->docTE)->clikLoop = (ProcPtr)AsmClikLoop;
      }

      if (good) { /* good document? -- get scrollbars */
        doc->docVScroll = GetNewControl(rVScroll, window);
        good = (doc->docVScroll != nil);
      }
      if (good) {
        doc->docHScroll = GetNewControl(rHScroll, window);
        good = (doc->docHScroll != nil);
      }

      if (good) {
        AdjustScrollValues(doc, false);
        AdjustScrollbars(doc, true);
        return window;
      } else {
        DoCloseWindow(window); /* otherwise regret we ever created it... */
        AlertUser(eNoWindow);  /* and tell user */
        return NULL;
      }
    } else
      DisposePtr(storage); /* get rid of the storage if it is never used */
    return NULL;
  }
}

// Create a new untitled document window and show it.
void DoNew() {
  WindowPtr window = mkDocumentWindow(TwelfDocument);
  ShowWindow(window);
}

/* Close a window. This handles desk accessory and application windows. */

Boolean DoCloseWindow(WindowPtr window) {
  if (IsDAWindow(window))
    CloseDeskAcc(((WindowPeek)window)->windowKind);
  else if (IsAppWindow(window)) {
    TwelfWinPtr twin = (TwelfWinPtr)window;
    switch (twin->winType) {
      case TwelfWinDocument: {
        DocumentPeek doc = getDoc(window);
        TEHandle te = doc->docTE;
        if (te != nil) {
          TEDispose(te);
        }
        CloseWindow(window);

        if (doc->docType == TwelfOutput) {
          gOutputWindow = NULL;
        }

        DisposePtr((Ptr)window);
        gNumDocuments -= 1;
      } break;
      case TwelfWinAbout: {
        CloseAboutBox();
      } break;
    }
    return true;
  }
}

/* Gets called from our assembly language routine, AsmClickLoop, which is in
        turn called by the TEClick toolbox routine. Saves the windows clip
   region, sets it to the portRect, adjusts the scrollbar values to match the TE
   scroll amount, then restores the clip region. */

Boolean IsAppWindow(WindowPtr window) {
  short windowKind;

  if (window == nil)
    return false;
  else { /* application windows have windowKinds = userKind (8) */
    windowKind = ((WindowPeek)window)->windowKind;
    return (windowKind == userKind);
  }
} /*IsAppWindow*/

/* Check to see if a window belongs to a desk accessory. */

Boolean IsDAWindow(WindowPtr window) {
  if (window == nil)
    return false;
  else /* DA windows have negative windowKinds */
    return ((WindowPeek)window)->windowKind < 0;
} /*IsDAWindow*/

/*	Display an alert that tells the user an error occurred, then exit the
   program. This routine is used as an ultimate bail-out for serious errors that
   prohibit the continuation of the application. Errors that do not require the
   termination of the application should be handled in a different manner. Error
   checking and reporting has a place even in the simplest application. The
   error number is used to index an 'STR#' resource so that a relevant message
   can be displayed. */

void AlertUser(short error) {
  short itemHit;
  Str255 message;

  SetCursor(&qd.arrow);
  /* type Str255 is an array in MPW 3 */
  GetIndString(message, kErrStrings, error);
  ParamText(message, "", "", "");
  itemHit = Alert(rUserAlert, nil);
} /* AlertUser */

/* Draw the contents of an application window. */

void DrawWindow_(WindowPtr window, short depth) {
  SetPort(window);
  TwelfWinPtr twin = (TwelfWinPtr)window;
  switch (twin->winType) {
    case TwelfWinAbout: {
      AboutPtr adoc = getAboutDoc(window);
      EraseRect(&window->portRect);
      if (depth == 1) {
        DrawPicture(adoc->bwPic, &adoc->picRect);
      } else {
        DrawPicture(adoc->pic, &adoc->picRect);
      }
      TEUpdate(&window->portRect, adoc->te);
    } break;
    case TwelfWinDocument: {
      DocumentPtr doc = getDoc(window);
      EraseRect(&window->portRect);
      DrawControls(window);
      DrawGrowIcon(window);
      TEUpdate(&window->portRect, doc->docTE);
    } break;
  }
}

pascal void DrawWindowWrap(short depth, short flags, GDHandle device,
                           long userData) {
  DrawWindow_((WindowPtr)userData, depth);
}

void DrawWindow(WindowPtr window) {
  DeviceLoop(window->visRgn, DrawWindowWrap, (long)(window), 0);
}

void Terminate() {
  do {
    WindowPtr window = FrontWindow(); /* get the current front window */
    if (window == nil) {
      break;
    }
    if (!(closeConfirmForWin(window) && DoCloseWindow(window))) {
      /* cancelled, don't terminate anymore */
      return;
    }
  } while (1);

  gRunning = false;
}

void alertSideEffect(char *message) {
  size_t len = strlen(message);
  if (len > 255) len = 255;
  char pMessage[256];
  pMessage[0] = len;
  strncpy(pMessage + 1, message, len);
  // display an alert
  ParamText(pMessage, "\p", "\p", "\p");
  SetCursor(&qd.arrow);
  Alert(rUserMessage, nil);
}
