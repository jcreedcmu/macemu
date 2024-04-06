#include "framework.h"

#include <Quickdraw.h>
#include <Traps.h>
#include <stdio.h>

#include "api.h"
#include "consts.h"
#include "events.h"
#include "framework.h"
#include "global-state.h"
#include "resource-consts.h"
#include "windows.h"

pascal OSErr MyHandleQuit(AppleEvent msg, AppleEvent reply, long refCon) {
  gRunning = false;
  return noErr;
}

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
}

void BigBadError(short error) {
  AlertUser(error);
  ExitToShell();
}

void Initialize() {
  Handle menuBar;
  long total, contig;
  EventRecord event;
  short count;

  gInBackground = false;
  gRunning = true;

  InitGraf(&qd.thePort);
  InitFonts();
  InitWindows();
  InitMenus();
  TEInit();
  InitDialogs(nil);
  InitCursor();

  // Install Apple Event handler
  short err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
                                    (AEEventHandlerUPP)MyHandleQuit, 0, false);
  if (err != 0) {
    printf("AEInstall err: %d\r", err);
  }

  for (count = 1; count <= 3; count++) EventAvail(everyEvent, &event);

  SysEnvirons(kSysEnvironsVersion, &gMac);

  if (gMac.machineType < 0) BigBadError(eWrongMachine);

  gHasWaitNextEvent = TrapAvailable(_WaitNextEvent, ToolTrap);

#if UNIVERSAL_INTERFACE
  if ((long)GetApplLimit() - (long)ApplicationZone() < kMinHeap)
    BigBadError(eSmallSize);
#endif

  PurgeSpace(&total, &contig);
  if (total < kMinSpace)
    if (UnloadScrap() != noErr)
      BigBadError(eNoMemory);
    else {
      PurgeSpace(&total, &contig);
      if (total < kMinSpace) BigBadError(eNoMemory);
    }

  menuBar = GetNewMBar(rMenuBar); /* read menus into menu bar */
  if (menuBar == nil) BigBadError(eNoMemory);
  SetMenuBar(menuBar); /* install menus */
  DisposeHandle(menuBar);
  AppendResMenu(GetMenuHandle(mApple), 'DRVR'); /* add DA names to Apple menu */
  DrawMenuBar();

  gNumDocuments = 0;

  DoNew(); /* create a single empty document */
}

int FrameworkEntry() {
  MaxApplZone();

  Initialize();
  // UnloadSeg((Ptr)Initialize); // Initialize must not be in Main segment

  EventLoop();

  ExitToShell();
}
