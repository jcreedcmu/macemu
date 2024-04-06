#include "events.h"

#include <Events.h>
#include <Types.h>

#include "consts.h"
#include "dialogs.h"
#include "global-state.h"
#include "handlers.h"
#include "windows.h"

void AdjustMenus(void);
void DoMenuCommand(long menuResult);

/* Do the right thing for an event. Determine what kind of event it is, and call
 the appropriate routines. */

void DoEvent(EventRecord *event) {
  short part, err;
  WindowPtr window;
  char key;
  Point aPoint;

  switch (event->what) {
    case nullEvent:
      /* we idle for null/mouse moved events ands for events which aren't
              ours (see EventLoop) */
      DoIdle();
      break;
    case mouseDown:
      part = FindWindow(event->where, &window);
      switch (part) {
        case inMenuBar:  /* process a mouse menu command (if any) */
          AdjustMenus(); /* bring 'em up-to-date */
          DoMenuCommand(MenuSelect(event->where));
          break;
        case inSysWindow: /* let the system handle the mouseDown */
          SystemClick(event, window);
          break;
        case inContent:
          if (window != FrontWindow()) {
            SelectWindow(window);
            /*DoEvent(event);*/ /* use this line for "do first click" */
          } else
            DoContentClick(window, event);
          break;
        case inDrag: /* pass screenBits.bounds to get all gDevices */
          DragWindow(window, event->where, &qd.screenBits.bounds);
          break;
        case inGoAway: {
          if (TrackGoAway(window, event->where)) {
            if (closeConfirmForDoc(getDoc(window))) {
              DoCloseWindow(window);
            }
          }
        } break;
        case inGrow:
          DoGrowWindow(window, event);
          break;
        case inZoomIn:
        case inZoomOut:
          if (TrackBox(window, event->where, part)) DoZoomWindow(window, part);
          break;
      }
      break;
    case keyDown:
    case autoKey: /* check for menukey equivalents */
      key = event->message & charCodeMask;
      if (event->modifiers & cmdKey) { /* Command key down */
        if (event->what == keyDown) {
          AdjustMenus(); /* enable/disable/check menu items properly */
          DoMenuCommand(MenuKey(key));
        }
      } else
        DoKeyDown(event);
      break;
    case activateEvt:
      DoActivate((WindowPtr)event->message,
                 (event->modifiers & activeFlag) != 0);
      break;
    case updateEvt:
      DoUpdate((WindowPtr)event->message);
      break;
    /*	1.01 - It is not a bad idea to at least call DIBadMount in response
            to a diskEvt, so that the user can format a floppy. */
    case diskEvt:

      if (HiWord(event->message) != noErr) {
        SetPt(&aPoint, kDILeft, kDITop);
        err = DIBadMount(aPoint, event->message);
      }

      break;
    case kHighLevelEvent:
      DoHighLevelEvent(event);
      break;
    case kOSEvent:
      /*	1.02 - must BitAND with 0x0FF to get only low byte */
      switch ((event->message >> 24) & 0x0FF) { /* high byte of message */
        case kMouseMovedMessage:
          DoIdle(); /* mouse-moved is also an idle event */
          break;
        case kSuspendResumeMessage: /* suspend/resume is also an
                                       activate/deactivate */
          gInBackground = (event->message & kResumeMask) == 0;
          DoActivate(FrontWindow(), !gInBackground);
          break;
      }
      break;
  }
}

/* Get events forever, and handle them by calling DoEvent.
   Also call AdjustCursor each time through the loop. */

void EventLoop() {
  RgnHandle cursorRgn;
  Boolean gotEvent;
  EventRecord event;
  Point mouse;

  cursorRgn = NewRgn(); /* we'll pass WNE an empty region the 1st time thru */
  do {
    /* use WNE if it is available */
    if (gHasWaitNextEvent) {
      GetGlobalMouse(&mouse);
      AdjustCursor(mouse, cursorRgn);
      gotEvent = WaitNextEvent(everyEvent, &event, GetSleep(), cursorRgn);
    } else {
      SystemTask();
      gotEvent = GetNextEvent(everyEvent, &event);
    }
    if (gotEvent) {
      /* make sure we have the right cursor before handling the event */
      AdjustCursor(event.where, cursorRgn);
      DoEvent(&event);
    } else
      DoIdle();   /* perform idle tasks when it's not our event */
                  /*	If you are using modeless dialogs that have editText items,
                          you will want to call IsDialogEvent to give the caret a chance
                          to blink, even if WNE/GNE returned FALSE. However, check FrontWindow
                          for a non-NIL value before calling IsDialogEvent. */
  } while (true); /* loop forever; we quit via ExitToShell */
} /*EventLoop*/
