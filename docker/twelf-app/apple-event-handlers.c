#include "apple-event-handlers.h"

#include <Events.h>
#include <stdio.h>

#include "global-state.h"

pascal OSErr handleAEQuit(AppleEvent msg, AppleEvent reply, long refCon) {
  gRunning = false;
  return noErr;
}

void installAEHandlers() {
  short err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
                                    (AEEventHandlerUPP)handleAEQuit, 0, false);
  if (err != 0) {
    printf("AEInstall Quit err: %d\r", err);
  }
}
