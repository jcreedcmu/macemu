#include "apple-event-handlers.h"

#include <Events.h>
#include <stdio.h>

#include "file-ops.h"
#include "global-state.h"

pascal OSErr handleAEOpen(AppleEvent *msg, AppleEvent *reply, long refCon) {
  AEDescList docList;
  printf("handleAEOpen invoked!\r");
  OSErr err;
  err = AEGetParamDesc(msg, keyDirectObject, typeAEList, &docList);
  if (err != 0) {
    printf("GetParamDesc error %d\r", err);
    return err;
  }
  long numItems;
  err = AECountItems(&docList, &numItems);
  if (err != 0) {
    printf("AECountItems error %d\r", err);
    return err;
  }

  for (long i = 0; i < numItems; i++) {
    FSSpec spec;
    size_t actualSize;
    DescType returnedType;
    AEKeyword keywd;
    err = AEGetNthPtr(&docList, i + 1, typeFSS, &keywd, &returnedType, &spec,
                      sizeof(FSSpec), &actualSize);

    openFileSpec(&spec);
  }

  AEDisposeDesc(docList);
  return noErr;
}

pascal OSErr handleAEQuit(AppleEvent *msg, AppleEvent *reply, long refCon) {
  gRunning = false;
  return noErr;
}

void installAEHandlers() {
  OSErr err;

  err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
                              (AEEventHandlerUPP)handleAEQuit, 0, false);
  if (err != 0) {
    printf("AEInstall Quit err: %d\r", err);
  }

  err = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
                              (AEEventHandlerUPP)handleAEOpen, 0, false);
  if (err != 0) {
    printf("AEInstall Open err: %d\r", err);
  }
}
