#include "handlers.h"

#include <limits.h>

#include "api.h"
#include "consts.h"
#include "document.h"
#include "global-state.h"
#include "rect-macros.h"
#include "scrolling.h"
#include "view-rects.h"
#include "windows.h"

/*	Get the global coordinates of the mouse. When you call OSEventAvail
        it will return either a pending event or a null event. In either case,
        the where field of the event record will contain the current position
        of the mouse in global coordinates and the modifiers field will reflect
        the current state of the modifiers. Another way to get the global
        coordinates is to call GetMouse and LocalToGlobal, but that requires
        being sure that thePort is set to a valid port. */

void GetGlobalMouse(Point *mouse) {
  EventRecord event;

  OSEventAvail(kNoEvents, &event); /* we aren't interested in any events */
  *mouse = event.where;            /* just the mouse position */
} /*GetGlobalMouse*/

/*	Called when a mouseDown occurs in the grow box of an active window. In
        order to eliminate any 'flicker', we want to invalidate only what is
        necessary. Since _ResizeWindow invalidates the whole portRect, we save
        the old TE viewRect, intersect it with the new TE viewRect, and
        remove the result from the update region. However, we must make sure
        that any old update region that might have been around gets put back. */

void DoGrowWindow(WindowPtr window, EventRecord *event) {
  long growResult;
  Rect tempRect;
  RgnHandle tempRgn;
  DocumentPeek doc;

  tempRect = qd.screenBits.bounds; /* set up limiting values */
  tempRect.left = kMinDocDim;
  tempRect.top = kMinDocDim;
  growResult = GrowWindow(window, event->where, &tempRect);
  /* see if it really changed size */
  if (growResult != 0) {
    doc = (DocumentPeek)window;
    tempRect = (*doc->docTE)->viewRect; /* save old text box */
    tempRgn = NewRgn();
    GetLocalUpdateRgn(window, tempRgn); /* get localized update region */
    SizeWindow(window, LoWord(growResult), HiWord(growResult), true);
    _ResizeWindow(window);
    /* calculate & validate the region that hasn't changed so it won't get
     * redrawn */
    SectRect(&tempRect, &(*doc->docTE)->viewRect, &tempRect);
    ValidRect(&tempRect); /* take it out of update */
    InvalRgn(tempRgn);    /* put back any prior update */
    DisposeRgn(tempRgn);
  }
} /* DoGrowWindow */

/* 	Called when a mouseClick occurs in the zoom box of an active window.
        Everything has to get re-drawn here, so we don't mind that
        _ResizeWindow invalidates the whole portRect. */

void DoZoomWindow(WindowPtr window, short part) {
  TwelfWinPtr twin = (TwelfWinPtr)window;
  switch (twin->winType) {
    case TwelfWinDocument: {
      EraseRect(&window->portRect);
      ZoomWindow(window, part, window == FrontWindow());
      _ResizeWindow(window);
    } break;
    case TwelfWinAbout: {
      /* Unimplemented */
    } break;
  }
} /*  DoZoomWindow */

/* Called when the window has been resized to fix up the controls and content.
 */
void _ResizeWindow(WindowPtr window) {
  TwelfWinPtr twin = (TwelfWinPtr)window;
  switch (twin->winType) {
    case TwelfWinDocument: {
      DocumentPtr doc = getDoc(window);
      AdjustScrollbars(doc, true);
      AdjustTE(doc);
      InvalRect(&window->portRect);
    } break;
    case TwelfWinAbout: {
      /* Unimplemented */
    } break;
  }
} /* _ResizeWindow */

/* Returns the update region in local coordinates */
void GetLocalUpdateRgn(WindowPtr window, RgnHandle localRgn) {
  CopyRgn(((WindowPeek)window)->updateRgn,
          localRgn); /* save old update region */
  OffsetRgn(localRgn, window->portBits.bounds.left,
            window->portBits.bounds.top);
} /* GetLocalUpdateRgn */

/*	This is called when an update event is received for a window.
        It calls DrawWindow to draw the contents of an application window.
        As an efficiency measure that does not have to be followed, it
        calls the drawing routine only if the visRgn is non-empty. This
        will handle situations where calculations for drawing or drawing
        itself is very time-consuming. */

void DoUpdate(WindowPtr window) {
  if (IsAppWindow(window)) {
    BeginUpdate(window);           /* this sets up the visRgn */
    if (!EmptyRgn(window->visRgn)) /* draw if updating needs to be done */
      DrawWindow(window);
    EndUpdate(window);
  }
} /*DoUpdate*/

/*	This is called when a window is activated or deactivated.
        It calls TextEdit to deal with the selection. */

void DoActivate(WindowPtr window, Boolean becomingActive) {
  RgnHandle tempRgn, clipRgn;
  Rect growRect;
  DocumentPeek doc;

  if (IsAppWindow(window)) {
    TwelfWinPtr twin = (TwelfWinPtr)window;
    switch (twin->winType) {
      case TwelfWinDocument: {
        doc = (DocumentPeek)window;
        if (becomingActive) {
          /*	since we don't want TEActivate to draw a selection in an area
             where we're going to erase and redraw, we'll clip out the update
             region before calling it. */
          tempRgn = NewRgn();
          clipRgn = NewRgn();
          GetLocalUpdateRgn(window, tempRgn); /* get localized update region */
          GetClip(clipRgn);
          DiffRgn(clipRgn, tempRgn,
                  tempRgn); /* subtract updateRgn from clipRgn */
          SetClip(tempRgn);
          TEActivate(doc->docTE);
          SetClip(clipRgn); /* restore the full-blown clipRgn */
          DisposeRgn(tempRgn);
          DisposeRgn(clipRgn);

          /* the controls must be redrawn on activation: */
          (*doc->docVScroll)->contrlVis = kControlVisible;
          (*doc->docHScroll)->contrlVis = kControlVisible;
          InvalRect(&(*doc->docVScroll)->contrlRect);
          InvalRect(&(*doc->docHScroll)->contrlRect);
          /* the growbox needs to be redrawn on activation: */
          growRect = window->portRect;
          /* adjust for the scrollbars */
          growRect.top = growRect.bottom - kScrollbarAdjust;
          growRect.left = growRect.right - kScrollbarAdjust;
          InvalRect(&growRect);
        } else {
          TEDeactivate(doc->docTE);
          /* the controls must be hidden on deactivation: */
          HideControl(doc->docVScroll);
          HideControl(doc->docHScroll);
          /* the growbox should be changed immediately on deactivation: */
          DrawGrowIcon(window);
        }
      } break;
      case TwelfWinAbout: {
        /* Nothing special to do */
      } break;
    }
  }
}

/*	This is called when a mouseDown occurs in the content of a
 * window. */

void DoContentClick(WindowPtr window, EventRecord *event) {
  Point mouse;
  ControlHandle control;
  short part, value;
  Boolean shiftDown;
  DocumentPeek doc;
  Rect teRect;

  if (IsAppWindow(window)) {
    TwelfWinPtr twin = (TwelfWinPtr)window;
    switch (twin->winType) {
      case TwelfWinDocument: {
        SetPort(window);
        mouse = event->where; /* get the click position */
        GlobalToLocal(&mouse);
        doc = (DocumentPeek)window;
        /* see if we are in the viewRect. if so, we won't check the
         * controls */
        GetTERect(window, &teRect);
        if (PtInRect(mouse, &teRect)) {
          /* see if we need to extend the selection */
          shiftDown =
              (event->modifiers & shiftKey) != 0; /* extend if Shift is down */
          TEClick(mouse, shiftDown, doc->docTE);
        } else {
          part = FindControl(mouse, window, &control);
          switch (part) {
            case 0: /* do nothing for viewRect case */
              break;
            case kControlIndicatorPart:
              value = GetControlValue(control);
              part = TrackControl(control, mouse, nil);
              if (part != 0) {
                value -= GetControlValue(control);
                /* value now has CHANGE in value; if value changed, scroll
                 */
                if (value != 0)
                  if (control == doc->docVScroll)
                    TEScroll(0, value * (*doc->docTE)->lineHeight, doc->docTE);
                  else
                    TEScroll(value, 0, doc->docTE);
              }
              break;
            default: /* they clicked in an arrow, so track & scroll */
            {
              if (control == doc->docVScroll) {
                value = TrackControl(control, mouse, VActionProc);
              } else if (control == doc->docHScroll) {
                value = TrackControl(control, mouse, HActionProc);
              }

            } break;
          }
        }
      } break;
      case TwelfWinAbout: {
      } break;
    }
  }
}

/* This is called for any keyDown or autoKey events, except when the
 Command key is held down. It looks at the frontmost window to decide what
 to do with the key typed. */

void DoKeyDown(EventRecord *event) {
  WindowPtr window;
  char key;
  TEHandle te;

  window = FrontWindow();
  if (IsAppWindow(window)) {
    TwelfWinPtr twin = (TwelfWinPtr)window;
    switch (twin->winType) {
      case TwelfWinDocument: {
        DocumentPeek doc = getDoc(window);
        te = doc->docTE;
        key = event->message & charCodeMask;
        /* we have a char. for our window; see if we are still below
           TextEdit's limit for the number of characters (but deletes are
           always rad) */
        if (key == kDelChar ||
            (*te)->teLength - ((*te)->selEnd - (*te)->selStart) + 1 <
                kMaxTELength) {
          doc->dirty = true;
          TEKey(key, te);
          AdjustScrollbars(doc, false);
          AdjustTE(doc);
        } else
          AlertUser(eExceedChar);
      } break;
      case TwelfWinAbout: {
        /* Nothing special to do */
      } break;
    }
  }
}

/*	Calculate a sleep value for WaitNextEvent. This takes into
   account the things that DoIdle does with idle time. */

unsigned long GetSleep() {
  WindowPtr window;

  if (!gInBackground) {
    window = FrontWindow(); /* and the front window is ours... */
    if (IsAppWindow(window)) {
      TwelfWinPtr twin = (TwelfWinPtr)window;
      switch (twin->winType) {
        case TwelfWinDocument: { /* and it's a proper document ... */
          TEHandle te =
              ((DocumentPeek)(window))->docTE; /* and the selection is an
                                                  insertion point... */
          if ((*te)->selStart == (*te)->selEnd) {
            return 250;  // GetCaretTime();
          }
        } break;
        case TwelfWinAbout: {
          return LONG_MAX;  // Is this correct?
        } break;
      }
    }
  }
  return LONG_MAX;
}

/* This is called whenever we get a null event et al.
 It takes care of necessary periodic actions. For this program, it calls
 TEIdle.
 */

void DoIdle() {
  WindowPtr window;
  window = FrontWindow();
  if (IsAppWindow(window)) {
    TwelfWinPtr twin = (TwelfWinPtr)window;
    switch (twin->winType) {
      case TwelfWinAbout: {
        // Conceivably some kind of idle animation could go here
      } break;
      case TwelfWinDocument: {
        TEIdle(((DocumentPeek)window)->docTE);
      } break;
    }
  }
}

/*	Change the cursor's shape, depending on its position. This also
   calculates the region where the current cursor resides (for
   WaitNextEvent). When the mouse moves outside of this region, an event
   is generated. If there is more to the event than just `the mouse
   moved', we get called before the event is processed to make sure the
   cursor is the right one. In any (ahem) event, this is called again
   before we fall back into WNE. */

void AdjustCursor(Point mouse, RgnHandle region) {
  WindowPtr window;
  RgnHandle arrowRgn;
  RgnHandle iBeamRgn;
  Rect iBeamRect;

  window = FrontWindow(); /* we only adjust the cursor when we are in front */
  if ((!gInBackground) && (!IsDAWindow(window))) {
    /* calculate regions for different cursor shapes */
    arrowRgn = NewRgn();
    iBeamRgn = NewRgn();

    /* start arrowRgn wide open */
    SetRectRgn(arrowRgn, kExtremeNeg, kExtremeNeg, kExtremePos, kExtremePos);

    /* calculate iBeamRgn */
    if (IsAppWindow(window)) {
      TwelfWinPtr twin = (TwelfWinPtr)window;
      switch (twin->winType) {
        case TwelfWinDocument: {
          iBeamRect = (*((DocumentPeek)window)->docTE)->viewRect;
          SetPort(window); /* make a global version of the viewRect */
          LocalToGlobal(&TopLeft(iBeamRect));
          LocalToGlobal(&BotRight(iBeamRect));
          RectRgn(iBeamRgn, &iBeamRect);
          /* we temporarily change the port's origin to 'globalify' the
           * visRgn
           */
          SetOrigin(-window->portBits.bounds.left,
                    -window->portBits.bounds.top);
          SectRgn(iBeamRgn, window->visRgn, iBeamRgn);
          SetOrigin(0, 0);
        } break;
        case TwelfWinAbout: {
          // no iBeamRgn for about box
        } break;
      }
    }

    /* subtract other regions from arrowRgn */
    DiffRgn(arrowRgn, iBeamRgn, arrowRgn);

    /* change the cursor and the region parameter */
    if (PtInRgn(mouse, iBeamRgn)) {
      SetCursor(*GetCursor(iBeamCursor));
      CopyRgn(iBeamRgn, region);
    } else {
      SetCursor(&qd.arrow);
      CopyRgn(arrowRgn, region);
    }

    DisposeRgn(arrowRgn);
    DisposeRgn(iBeamRgn);
  }
} /*AdjustCursor*/

void DoHighLevelEvent(EventRecord *event) {
  ////
  AEProcessAppleEvent(event);
}
