// We are trying to compile with multiversal interface
#define UNIVERSAL_INTERFACE 0

#include <Devices.h>
#include <Dialogs.h>
#include <Events.h>
#include <Files.h>
#include <Fonts.h>
#include <Memory.h>
#include <Menus.h>
#include <OSUtils.h>
#include <Quickdraw.h>
#include <SegLoad.h>
#include <TextEdit.h>
#include <ToolUtils.h>
#include <Traps.h>
#include <Types.h>
#include <Windows.h>
#include <limits.h>
#include <stdio.h>

#include "api.h"
#include "asmclikloop.h"
#include "clikloop.h"
#include "consts.h"
#include "document.h"
#include "events.h"
#include "global-state.h"
#include "handlers.h"
#include "resource-consts.h"
#include "scrolling.h"
#include "view-rects.h"

#if UNIVERSAL_INTERFACE
#include <ControlDefinitions.h>
#include <Controls.h>
#include <DiskInit.h>
#include <Packages.h>
#include <Scrap.h>
#else
#include "scrap.h"
#endif

Boolean IsAppWindow(WindowPtr window);
void AlertUser(short error);
void AdjustMenus(void);
void DoMenuCommand(long menuResult);
void DoNew(void);
void Terminate(void);
void Initialize(void);
void BigBadError(short error);
Boolean IsDAWindow(WindowPtr window);
Boolean TrapAvailable(short tNumber, TrapType tType);

/* Define TopLeft and BotRight macros for convenience. Notice the implicit
   dependency on the ordering of fields within a Rect */
#define TopLeft(aRect) (*(Point *)&(aRect).top)
#define BotRight(aRect) (*(Point *)&(aRect).bottom)

/*	Set up the whole world, including global variables, Toolbox managers,
        menus, and a single blank document. */

/*	1.01 - The code that used to be part of ForceEnvirons has been moved
   into this module. If an error is detected, instead of merely doing an
   ExitToShell, which leaves the user without much to go on, we call AlertUser,
   which puts up a simple alert that just says an error occurred and then calls
   ExitToShell. Since there is no other cleanup needed at this point if an error
   is detected, this form of error- handling is acceptable. If more
   sophisticated error recovery is needed, an exception mechanism, such as is
   provided by Signals, can be used. */

void Initialize() {
  Handle menuBar;
  long total, contig;
  EventRecord event;
  short count;

  gInBackground = false;

  InitGraf(&qd.thePort);
  InitFonts();
  InitWindows();
  InitMenus();
  TEInit();
  InitDialogs(nil);
  InitCursor();

  /*	Call MPPOpen and ATPLoad at this point to initialize AppleTalk,
          if you are using it. */
  /*	NOTE -- It is no longer necessary, and actually unhealthy, to check
          PortBUse and SPConfig before opening AppleTalk. The drivers are
     capable of checking for port availability themselves. */

  /*	This next bit of code is necessary to allow the default button of our
          alert be outlined.
          1.02 - Changed to call EventAvail so that we don't lose some important
          events. */

  for (count = 1; count <= 3; count++) EventAvail(everyEvent, &event);

  /*	Ignore the error returned from SysEnvirons; even if an error occurred,
          the SysEnvirons glue will fill in the SysEnvRec. You can save a
     redundant call to SysEnvirons by calling it after initializing AppleTalk.
   */

  SysEnvirons(kSysEnvironsVersion, &gMac);

  /* Make sure that the machine has at least 128K ROMs. If it doesn't, exit. */

  if (gMac.machineType < 0) BigBadError(eWrongMachine);

  /*	1.02 - Move TrapAvailable call to after SysEnvirons so that we can tell
          in TrapAvailable if a tool trap value is out of range. */

  gHasWaitNextEvent = TrapAvailable(_WaitNextEvent, ToolTrap);

  /*	1.01 - We used to make a check for memory at this point by examining
     ApplLimit, ApplicationZone, and StackSpace and comparing that to the
     minimum size we told MultiFinder we needed. This did not work well because
     it assumed too much about the relationship between what we asked
     MultiFinder for and what we would actually get back, as well as how to
     measure it. Instead, we will use an alternate method comprised of two
     steps. */

  /*	It is better to first check the size of the application heap against a
     value that you have determined is the smallest heap the application can
     reasonably work in. This number should be derived by examining the size of
     the heap that is actually provided by MultiFinder when the minimum size
     requested is used. The derivation of the minimum size requested from
     MultiFinder is described in Sample.h. The check should be made because the
     preferred size can end up being set smaller than the minimum size by the
     user. This extra check acts to insure that your application is starting
     from a solid memory foundation. */

#if UNIVERSAL_INTERFACE
  if ((long)GetApplLimit() - (long)ApplicationZone() < kMinHeap)
    BigBadError(eSmallSize);
#endif

  /*	Next, make sure that enough memory is free for your application to run.
     It is possible for a situation to arise where the heap may have been of
     required size, but a large scrap was loaded which left too little memory.
     To check for this, call PurgeSpace and compare the result with a value that
     you have determined is the minimum amount of free memory your application
     needs at initialization. This number can be derived several different ways.
     One way that is fairly straightforward is to run the application in the
     minimum size configuration as described previously. Call PurgeSpace at
     initialization and examine the value returned. However, you should make
     sure that this result is not being modified by the scrap's presence. You
     can do that by calling ZeroScrap before calling PurgeSpace. Make sure to
     remove that call before shipping, though. */

  /* ZeroScrap(); */

  PurgeSpace(&total, &contig);
  if (total < kMinSpace)
    if (UnloadScrap() != noErr)
      BigBadError(eNoMemory);
    else {
      PurgeSpace(&total, &contig);
      if (total < kMinSpace) BigBadError(eNoMemory);
    }

  /*	The extra benefit to waiting until after the Toolbox Managers have been
     initialized to check memory is that we can now give the user an alert to
     tell him/her what happened. Although it is possible that the memory
     situation could be worsened by displaying an alert, MultiFinder would
     gracefully exit the application with an informative alert if memory became
     critical. Here we are acting more in a preventative manner to avoid future
     disaster from low-memory problems. */

  menuBar = GetNewMBar(rMenuBar); /* read menus into menu bar */
  if (menuBar == nil) BigBadError(eNoMemory);
  SetMenuBar(menuBar); /* install menus */
  DisposeHandle(menuBar);
  AppendResMenu(GetMenuHandle(mApple), 'DRVR'); /* add DA names to Apple menu */
  DrawMenuBar();

  gNumDocuments = 0;

  /* do other initialization here */

  DoNew(); /* create a single empty document */
} /*Initialize*/

/* Used whenever a, like, fully fatal error happens */
void BigBadError(short error) {
  AlertUser(error);
  ExitToShell();
}

/*	Check to see if a given trap is implemented. This is only used by the
        Initialize routine in this program, so we put it in the Initialize
   segment. The recommended approach to see if a trap is implemented is to see
   if the address of the trap routine is the same as the address of the
        Unimplemented trap. */
/*	1.02 - Needs to be called after call to SysEnvirons so that it can check
        if a ToolTrap is out of range of a pre-MacII ROM. */

Boolean TrapAvailable(short tNumber, TrapType tType) {
  if ((tType == (unsigned char)ToolTrap) &&
      (gMac.machineType > envMachUnknown) &&
      (gMac.machineType < envMacII)) { /* it's a 512KE, Plus, or SE */
    tNumber = tNumber & 0x03FF;
    if (tNumber > 0x01FF)       /* which means the tool traps */
      tNumber = _Unimplemented; /* only go to 0x01FF */
  }
  return NGetTrapAddress(tNumber, tType) !=
         NGetTrapAddress(_Unimplemented, ToolTrap);
} /*TrapAvailable*/

int main() {
  // Debugging Log
  stdout = stderr = fopen("out", "w");
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  /* 1.01 - call to ForceEnvirons removed */

  /*	If you have stack requirements that differ from the default,
          then you could use SetApplLimit to increase StackSpace at
          this point, before calling MaxApplZone. */
  MaxApplZone(); /* expand the heap so code segments load at the top */

  Initialize();               /* initialize the program */
  UnloadSeg((Ptr)Initialize); /* note that Initialize must not be in Main! */

  EventLoop(); /* call the main event loop */
}

/*	Change the cursor's shape, depending on its position. This also
   calculates the region where the current cursor resides (for WaitNextEvent).
   When the mouse moves outside of this region, an event is generated. If there
   is more to the event than just `the mouse moved', we get called before the
   event is processed to make sure the cursor is the right one. In any (ahem)
   event, this is called again before we fall back into WNE. */

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
      iBeamRect = (*((DocumentPeek)window)->docTE)->viewRect;
      SetPort(window); /* make a global version of the viewRect */
      LocalToGlobal(&TopLeft(iBeamRect));
      LocalToGlobal(&BotRight(iBeamRect));
      RectRgn(iBeamRgn, &iBeamRect);
      /* we temporarily change the port's origin to 'globalify' the visRgn */
      SetOrigin(-window->portBits.bounds.left, -window->portBits.bounds.top);
      SectRgn(iBeamRgn, window->visRgn, iBeamRgn);
      SetOrigin(0, 0);
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

/* Draw the contents of an application window. */

void DrawWindow(WindowPtr window) {
  SetPort(window);
  EraseRect(&window->portRect);
  DrawControls(window);
  DrawGrowIcon(window);
  TEUpdate(&window->portRect, ((DocumentPeek)window)->docTE);
} /*DrawWindow*/

/*	Enable and disable menus based on the current state.
        The user can only select enabled menu items. We set up all the menu
   items before calling MenuSelect or MenuKey, since these are the only times
   that a menu item can be selected. Note that MenuSelect is also the only time
        the user will see menu items. This approach to deciding what enable/
        disable state a menu item has the advantage of concentrating all
        the decision-making in one routine, as opposed to being spread
   throughout the application. Other application designs may take a different
   approach that may or may not be as valid. */

void AdjustMenus() {
  WindowPtr window;
  MenuHandle menu;
  long offset;
  Boolean undo;
  Boolean cutCopyClear;
  Boolean paste;
  TEHandle te;

  window = FrontWindow();

  menu = GetMenuHandle(mFile);
  if (gNumDocuments < kMaxOpenDocuments)
    EnableItem(menu, iNew); /* New is enabled when we can open more documents */
  else
    DisableItem(menu, iNew);
  if (window != nil) /* Close is enabled when there is a window to close */
    EnableItem(menu, iClose);
  else
    DisableItem(menu, iClose);

  menu = GetMenuHandle(mEdit);
  undo = false;
  cutCopyClear = false;
  paste = false;
  if (IsDAWindow(window)) {
    undo = true; /* all editing is enabled for DA windows */
    cutCopyClear = true;
    paste = true;
  } else if (IsAppWindow(window)) {
    te = ((DocumentPeek)window)->docTE;
    if ((*te)->selStart < (*te)->selEnd) cutCopyClear = true;
    /* Cut, Copy, and Clear is enabled for app. windows with selections */
    if (GetScrap(nil, 'TEXT', &offset) > 0)
      paste = true; /* if there's any text in the clipboard, paste is enabled */
  }
  if (undo)
    EnableItem(menu, iUndo);
  else
    DisableItem(menu, iUndo);
  if (cutCopyClear) {
    EnableItem(menu, iCut);
    EnableItem(menu, iCopy);
    EnableItem(menu, iClear);
  } else {
    DisableItem(menu, iCut);
    DisableItem(menu, iCopy);
    DisableItem(menu, iClear);
  }
  if (paste)
    EnableItem(menu, iPaste);
  else
    DisableItem(menu, iPaste);
} /*AdjustMenus*/

/*	This is called when an item is chosen from the menu bar (after calling
        MenuSelect or MenuKey). It does the right thing for each command. */

void DoMenuCommand(long menuResult) {
  short menuID, menuItem;
  short itemHit, daRefNum;
  Str255 daName;
  OSErr saveErr;
  TEHandle te;
  WindowPtr window;
  Handle aHandle;
  long oldSize, newSize;
  long total, contig;

  window = FrontWindow();
  menuID = HiWord(menuResult);   /* use macros for efficiency to... */
  menuItem = LoWord(menuResult); /* get menu item number and menu number */
  switch (menuID) {
    case mApple:
      switch (menuItem) {
        case iAbout: /* bring up alert for About */
          itemHit = Alert(rAboutAlert, nil);
          break;
        default: /* all non-About items in this menu are DAs et al */
          /* type Str255 is an array in MPW 3 */
          GetMenuItemText(GetMenuHandle(mApple), menuItem, daName);
          daRefNum = OpenDeskAcc(daName);
          break;
      }
      break;
    case mFile:
      switch (menuItem) {
        case iNew:
          DoNew();
          break;
        case iClose:
          DoCloseWindow(FrontWindow()); /* ignore the result */
          break;
        case iQuit:
          Terminate();
          break;
      }
      break;
    case mEdit: /* call SystemEdit for DA editing & MultiFinder */
      if (!SystemEdit(menuItem - 1)) {
        te = ((DocumentPeek)FrontWindow())->docTE;
        switch (menuItem) {
          case iCut:
            if (ZeroScrap() == noErr) {
              PurgeSpace(&total, &contig);
              if ((*te)->selEnd - (*te)->selStart + kTESlop > contig)
                AlertUser(eNoSpaceCut);
              else {
                TECut(te);
                if (TEToScrap() != noErr) {
                  AlertUser(eNoCut);
                  ZeroScrap();
                }
              }
            }
            break;
          case iCopy:
            if (ZeroScrap() == noErr) {
              TECopy(te); /* after copying, export the TE scrap */
              if (TEToScrap() != noErr) {
                AlertUser(eNoCopy);
                ZeroScrap();
              }
            }
            break;
          case iPaste: /* import the TE scrap before pasting */
            if (TEFromScrap() == noErr) {
              if (TEGetScrapLength() +
                      ((*te)->teLength - ((*te)->selEnd - (*te)->selStart)) >
                  kMaxTELength)
                AlertUser(eExceedPaste);
              else {
                aHandle = (Handle)TEGetText(te);
                oldSize = GetHandleSize(aHandle);
                newSize = oldSize + TEGetScrapLength() + kTESlop;
                SetHandleSize(aHandle, newSize);
                saveErr = MemError();
                SetHandleSize(aHandle, oldSize);
                if (saveErr != noErr)
                  AlertUser(eNoSpacePaste);
                else
                  TEPaste(te);
              }
            } else
              AlertUser(eNoPaste);
            break;
          case iClear:
            TEDelete(te);
            break;
        }
        AdjustScrollbars(window, false);
        AdjustTE(window);
      }
      break;
  }
  HiliteMenu(0); /* unhighlight what MenuSelect (or MenuKey) hilited */
} /*DoMenuCommand*/

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

/**************************************************************************************
*** 1.01 DoCloseBehind(window) was removed ***

        1.01 - DoCloseBehind was a good idea for closing windows when quitting
        and not having to worry about updating the windows, but it suffered
        from a fatal flaw. If a desk accessory owned two windows, it would
        close both those windows when CloseDeskAcc was called. When
DoCloseBehind got around to calling DoCloseWindow for that other window that was
already closed, things would go very poorly. Another option would be to have a
        procedure, GetRearWindow, that would go through the window list and
return the last window. Instead, we decided to present the standard approach of
getting and closing FrontWindow until FrontWindow returns NIL. This has a
potential benefit in that the window whose document needs to be saved may be
visible since it is the front window, therefore decreasing the chance of user
confusion. For aesthetic reasons, the windows in the application should be
checked for updates periodically and have the updates serviced.
**************************************************************************************/

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
