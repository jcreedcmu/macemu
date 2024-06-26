#include "framework.h"

#include <Quickdraw.h>
#include <Traps.h>

#include "api.h"
#include "apple-event-handlers.h"
#include "buildinfo.h"
#include "consts.h"
#include "debug.h"
#include "events.h"
#include "framework.h"
#include "global-state.h"
#include "resource-consts.h"
#include "twelf.h"
#include "windows.h"

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

  installAEHandlers();

  for (count = 1; count <= 3; count++) EventAvail(everyEvent, &event);

  SysEnvirons(kSysEnvironsVersion, &gMac);

  if (gMac.machineType < 0) BigBadError(eWrongMachine);

  gHasWaitNextEvent = TrapAvailable(_WaitNextEvent, ToolTrap);

#if UNIVERSAL_INTERFACE
  if ((long)GetApplLimit() - (long)ApplicationZone() < kMinHeap)
    BigBadError(eSmallSize);
#endif

  PurgeSpace(&total, &contig);
  if (total < kMinSpace) {
    if (UnloadScrap() != noErr)
      BigBadError(eNoMemory);
    else {
      PurgeSpace(&total, &contig);
      if (total < kMinSpace) BigBadError(eNoMemory);
    }
  }

  menuBar = GetNewMBar(rMenuBar); /* read menus into menu bar */
  if (menuBar == nil) BigBadError(eNoMemory);
  SetMenuBar(menuBar); /* install menus */
  DisposeHandle(menuBar);
  AppendResMenu(GetMenuHandle(mApple), 'DRVR'); /* add DA names to Apple menu */
  DrawMenuBar();

  gNumDocuments = 0;

  // Make sure log window exists to receive stderr
  gLogWindow = getLogWindow();

  logger(build_info);
}

void FrameworkEntry() {
  MaxApplZone();

  Initialize();
  // UnloadSeg((Ptr)Initialize); // Initialize must not be in Main segment

  InitTwelf();

  EventLoop();

  ExitToShell();
}
