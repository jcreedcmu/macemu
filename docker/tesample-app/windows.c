#include "windows.h"

#include "api.h"
#include "asmclikloop.h"
#include "consts.h"
#include "document.h"
#include "global-state.h"
#include "resource-consts.h"
#include "scrolling.h"
#include "view-rects.h"

/* Create a new document and window. */

void DoNew() {
  Boolean good;
  Ptr storage;
  WindowPtr window;
  Rect destRect, viewRect;
  DocumentPeek doc;

  storage = NewPtr(sizeof(DocumentRecord));
  if (storage != nil) {
    window = GetNewWindow(rDocWindow, storage, (WindowPtr)-1);
    if (window != nil) {
      gNumDocuments +=
          1; /* this will be decremented when we call DoCloseWindow */
      good = false;
      SetPort(window);
      doc = (DocumentPeek)window;
      GetTERect(window, &viewRect);
      destRect = viewRect;
      destRect.right = destRect.left + kMaxDocWidth;
      doc->docTE = TENew(&destRect, &viewRect);
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

      if (good) { /* good? -- adjust & draw the controls, draw the window */
        /* false to AdjustScrollValues means musn't redraw; technically, of
        course, the window is hidden so it wouldn't matter whether we called
        ShowControl or not. */
        AdjustScrollValues(window, false);
        ShowWindow(window);
      } else {
        DoCloseWindow(window); /* otherwise regret we ever created it... */
        AlertUser(eNoWindow);  /* and tell user */
      }
    } else
      DisposePtr(storage); /* get rid of the storage if it is never used */
  }
} /*DoNew*/

/* Close a window. This handles desk accessory and application windows. */

/*	1.01 - At this point, if there was a document associated with a
        window, you could do any document saving processing if it is 'dirty'.
        DoCloseWindow would return true if the window actually closed, i.e.,
        the user didn't cancel from a save dialog. This result is handy when
        the user quits an application, but then cancels the save of a document
        associated with a window. */

Boolean DoCloseWindow(WindowPtr window) {
  TEHandle te;

  if (IsDAWindow(window))
    CloseDeskAcc(((WindowPeek)window)->windowKind);
  else if (IsAppWindow(window)) {
    te = ((DocumentPeek)window)->docTE;
    if (te != nil)
      TEDispose(te); /* dispose the TEHandle if we got far enough to make one */
    /*	1.01 - We used to call DisposeWindow, but that was technically
            incorrect, even though we allocated storage for the window on
            the heap. We should instead call CloseWindow to have the structures
            taken care of and then dispose of the storage ourselves. */
    CloseWindow(window);
    DisposePtr((Ptr)window);
    gNumDocuments -= 1;
  }
  return true;
} /*DoCloseWindow*/

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

void DrawWindow(WindowPtr window) {
  SetPort(window);
  EraseRect(&window->portRect);
  DrawControls(window);
  DrawGrowIcon(window);
  TEUpdate(&window->portRect, ((DocumentPeek)window)->docTE);
} /*DrawWindow*/

/* Clean up the application and exit. We close all of the windows so that
 they can update their documents, if any. */

/*	1.01 - If we find out that a cancel has occurred, we won't exit to the
        shell, but will return instead. */

void Terminate() {
  WindowPtr aWindow;
  Boolean closed;

  closed = true;
  do {
    aWindow = FrontWindow(); /* get the current front window */
    if (aWindow != nil) closed = DoCloseWindow(aWindow); /* close this window */
  } while (closed && (aWindow != nil));
  if (closed) ExitToShell(); /* exit if no cancellation */
} /*Terminate*/
