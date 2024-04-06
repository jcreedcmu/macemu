#include "clikloop.h"

#include <TextEdit.h>
#include <Windows.h>

#include "document.h"

pascal void PascalClikLoop() {
  WindowPtr window;
  RgnHandle region;

  window = FrontWindow();
  region = NewRgn();
  GetClip(region); /* save clip */
  ClipRect(&window->portRect);
  AdjustScrollValues(window, true); /* pass true for canRedraw */
  SetClip(region);                  /* restore clip */
  DisposeRgn(region);
} /* Pascal/C ClickLoop */

/* Gets called from our assembly language routine, AsmClickLoop, which is in
        turn called by the TEClick toolbox routine. It returns the address of
   the default clickLoop routine that was put into the TERec by TEAutoView to
        AsmClickLoop so that it can call it. */

pascal ProcPtr GetOldClikLoop() {
  return ((DocumentPeek)FrontWindow())->docClick;
} /* GetOldClickLoop */

/*	Check to see if a window belongs to the application. If the window
   pointer passed was NIL, then it could not be an application window.
   WindowKinds that are negative belong to the system and windowKinds less than
   userKind are reserved by Apple except for windowKinds equal to dialogKind,
   which mean it is a dialog. 1.02 - In order to reduce the chance of
   accidentally treating some window as an AppWindow that shouldn't be, we'll
   only return true if the windowkind is userKind. If you add different kinds of
   windows to Sample you'll need to change how this all works. */
