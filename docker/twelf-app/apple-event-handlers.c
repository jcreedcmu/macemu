#include "apple-event-handlers.h"

#include <Events.h>

#include "debug.h"
#include "file-ops.h"
#include "global-state.h"
#include "windows.h"

pascal OSErr handleAEOpenDocuments(AppleEvent *msg, AppleEvent *reply,
                                   long refCon) {
  AEDescList docList;
  logger("handleAEOpenDocs invoked!");
  OSErr err;
  err = AEGetParamDesc(msg, keyDirectObject, typeAEList, &docList);
  if (err != 0) {
    logger("GetParamDesc error %d", err);
    return err;
  }
  long numItems;
  err = AECountItems(&docList, &numItems);
  if (err != 0) {
    logger("AECountItems error %d", err);
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

  AEDisposeDesc(&docList);
  return noErr;
}

pascal OSErr handleAEOpenApplication(AppleEvent *msg, AppleEvent *reply,
                                     long refCon) {
  AEDescList docList;
  logger("handleAEOpenApp invoked!");
  DoNew();

  return noErr;
}

pascal OSErr handleAEQuit(AppleEvent *msg, AppleEvent *reply, long refCon) {
  // This has the effect of setting gRunning to false, not really
  // calling ExitToShell() yet.
  Terminate();

  return noErr;
}

void installAEHandlers() {
  OSErr err;

  err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
                              (AEEventHandlerUPP)handleAEQuit, 0, false);
  if (err != 0) {
    logger("AEInstall Quit err: %d", err);
  }

  err =
      AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
                            (AEEventHandlerUPP)handleAEOpenDocuments, 0, false);
  if (err != 0) {
    logger("AEInstall Open Doc err: %d", err);
  }

  err = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication,
                              (AEEventHandlerUPP)handleAEOpenApplication, 0,
                              false);
  if (err != 0) {
    logger("AEInstall Open App err: %d", err);
  }
}
