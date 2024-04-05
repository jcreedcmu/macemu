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
#include "windows.h"

#if UNIVERSAL_INTERFACE
#include <ControlDefinitions.h>
#include <Controls.h>
#include <DiskInit.h>
#include <Packages.h>
#include <Scrap.h>
#else
#include "scrap.h"
#endif

void Terminate(void);
void Initialize(void);
void BigBadError(short error);
Boolean TrapAvailable(short tNumber, TrapType tType);

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
