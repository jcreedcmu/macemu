#include "menus.h"

#include "api.h"
#include "consts.h"
#include "document.h"
#include "global-state.h"
#include "resource-consts.h"
#include "scrolling.h"
#include "windows.h"

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
  if (gNumDocuments < kMaxOpenDocuments) {
    /* New, Open enabled when we can open more documents */
    EnableItem(menu, iNew);
    EnableItem(menu, iOpen);
  } else {
    DisableItem(menu, iNew);
    DisableItem(menu, iOpen);
  }

  if (window != nil) {
    EnableItem(menu, iClose);
    EnableItem(menu, iSaveAs);
    DocumentPeek doc = (DocumentPeek)window;
    if (doc->fsSpecSet && doc->dirty) {
      EnableItem(menu, iSave);  // also Revert?
    } else {
      DisableItem(menu, iSave);
    }
  } else {
    DisableItem(menu, iClose);
    DisableItem(menu, iSave);
    DisableItem(menu, iSaveAs);
  }

  EnableItem(menu, iQuit);

  menu = GetMenuHandle(mEdit);
  undo = false;
  cutCopyClear = false;
  paste = false;
  Boolean selectAll = false;

  if (IsDAWindow(window)) {
    undo = true; /* all editing is enabled for DA windows */
    cutCopyClear = true;
    paste = true;
  } else if (IsAppWindow(window)) {
    selectAll = true;
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

  if (selectAll)
    EnableItem(menu, iSelectAll);
  else
    DisableItem(menu, iSelectAll);

} /*AdjustMenus*/

// FIXME(fs): better error handling in all file operations
OSErr writeFile(TEHandle te, FSSpec *spec) {
  OSErr err;
  short refNum;
  err = FSpOpenDF(spec, fsRdWrPerm, &refNum);
  Handle textHandle = (Handle)TEGetText(te);
  long size = (*te)->teLength;
  err = FSWrite(refNum, &size, *textHandle);
  err = FSClose(refNum);

  return err;
}

OSErr readFile(TEHandle te, FSSpec *spec) {
  int16_t refNum;
  long textLength;
  OSErr err = FSpOpenDF(spec, fsCurPerm, &refNum);
  err = SetFPos(refNum, fsFromStart, 0);
  err = GetEOF(refNum, &textLength);
  if (textLength > kMaxTELength) {
    textLength = kMaxTELength;
  }
  Handle buf = NewHandle(textLength);
  err = FSRead(refNum, &textLength, *buf);
  for (int i = 0; i < textLength; i++) {
    if ((*buf)[i] == '\n') {
      (*buf)[i] = '\r';
    }
  }
  MoveHHi(buf);
  HLock(buf);
  TESetText(*buf, textLength, te);
  HUnlock(buf);
  DisposeHandle(buf);
  err = FSClose(refNum);
  return err;
}

void associateFile(WindowPtr window, FSSpec *spec) {
  DocumentPeek doc = (DocumentPeek)window;
  doc->fsSpec = *spec;
  doc->fsSpecSet = true;
  SetWTitle(window, spec->name);
}

void DoSave(WindowPtr window) {
  DocumentPeek doc = (DocumentPeek)window;
  TEHandle te = doc->docTE;
  writeFile(doc->docTE, &doc->fsSpec);
  doc->dirty = false;
}

void DoSaveAs(WindowPtr window) {
  DocumentPeek doc = (DocumentPeek)window;
  TEHandle te = doc->docTE;
  StandardFileReply reply;
  StandardPutFile("\pSave as:", "\puntitled", &reply);
  if (!reply.sfGood) return;

  FSSpec *spec = &reply.sfFile;
  FSpCreate(spec, 'ttxt', 'TEXT', smSystemScript);
  writeFile(doc->docTE, spec);
  associateFile(window, spec);
  doc->dirty = false;
}

void DoOpen() {
  StandardFileReply reply;
  SFTypeList types = {'TEXT'};

  StandardGetFile(nil, 1, types, &reply);
  if (!reply.sfGood) return;

  WindowPtr window = mkDocumentWindow();
  if (window == NULL) return;
  DocumentPeek doc = (DocumentPeek)window;
  TEHandle te = doc->docTE;

  FSSpec *spec = &reply.sfFile;
  readFile(te, spec);

  associateFile(window, spec);

  ShowWindow(window);
  InvalRect(&window->portRect);
}

/*	This is called when an item is chosen from the menu bar (after calling
        MenuSelect or MenuKey). It does the right thing for each command. */

void DoMenuCommand(long menuResult) {
  short menuID, menuItem;
  short itemHit, daRefNum;
  Str255 daName;
  OSErr saveErr;
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
        case iOpen:
          DoOpen();
          break;
        case iClose:
          DoCloseWindow(FrontWindow()); /* ignore the result */
          break;
        case iSave: {
          DoSave(FrontWindow());
        } break;
        case iSaveAs: {
          DoSaveAs(FrontWindow());
        } break;
        case iQuit:
          Terminate();
          break;
      }
      break;
    case mEdit: { /* call SystemEdit for DA editing & MultiFinder */
      DocumentPeek doc = (DocumentPeek)FrontWindow();
      TEHandle te = doc->docTE;
      if (!SystemEdit(menuItem - 1)) {
        switch (menuItem) {
          case iCut:
            if (ZeroScrap() == noErr) {
              PurgeSpace(&total, &contig);
              if ((*te)->selEnd - (*te)->selStart + kTESlop > contig)
                AlertUser(eNoSpaceCut);
              else {
                TECut(te);
                doc->dirty = true;
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
                else {
                  TEPaste(te);
                  doc->dirty = true;
                }
              }
            } else
              AlertUser(eNoPaste);
            break;
          case iClear:
            TEDelete(te);
            doc->dirty = true;
            break;
          case iSelectAll: {
            TESetSelect(0, (*te)->teLength, te);
          } break;
        }
        AdjustScrollbars(window, false);
        AdjustTE(window);
      }
    } break;
  }
  HiliteMenu(0); /* unhighlight what MenuSelect (or MenuKey) hilited */
} /*DoMenuCommand*/
