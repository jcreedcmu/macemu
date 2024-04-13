#include "menus.h"

#include <libtwelf.h>
#include <string.h>

#include "about.h"
#include "api.h"
#include "console.h"
#include "consts.h"
#include "debug.h"
#include "dialogs.h"
#include "document.h"
#include "events.h"
#include "file-ops.h"
#include "global-state.h"
#include "handlers.h"
#include "multiversal-stubs.h"
#include "resource-consts.h"
#include "scrolling.h"
#include "windows.h"

void AdjustFileMenu() {
  WindowPtr window = FrontWindow();
  MenuHandle menu = GetMenuHandle(mFile);

  if (gNumDocuments < kMaxOpenDocuments) {
    /* New, Open enabled when we can open more documents */
    EnableItem(menu, iNew);
    EnableItem(menu, iOpen);
  } else {
    DisableItem(menu, iNew);
    DisableItem(menu, iOpen);
  }

  Boolean closeEnabled = false;
  Boolean saveEnabled = false;
  Boolean saveAsEnabled = false;

  if (window != nil) {
    closeEnabled = true;
    if (IsAppWindow(window)) {
      TwelfWinPtr twin = (TwelfWinPtr)window;
      switch (twin->winType) {
        case TwelfWinDocument: {
          DocumentPeek doc = getDoc(window);
          saveAsEnabled = true;
          if (doc->fsSpecSet && doc->dirty) {
            saveEnabled = true;  // maybe also revert?
          }
        } break;
        case TwelfWinAbout: {
        } break;
      }
    }
  }

  if (closeEnabled) {
    EnableItem(menu, iClose);
  } else {
    DisableItem(menu, iClose);
  }

  if (saveEnabled) {
    EnableItem(menu, iSave);
  } else {
    DisableItem(menu, iSave);
  }

  if (saveAsEnabled) {
    EnableItem(menu, iSaveAs);
  } else {
    DisableItem(menu, iSaveAs);
  }

  EnableItem(menu, iQuit);
}

void AdjustEditMenu() {
  WindowPtr window = FrontWindow();
  MenuHandle menu = GetMenuHandle(mEdit);

  Boolean undo = false;
  Boolean copy = false;
  Boolean cutClear = false;
  Boolean paste = false;
  Boolean selectAll = false;

  if (IsDAWindow(window)) {
    undo = true; /* all editing is enabled for DA windows */
    copy = true;
    cutClear = true;
    paste = true;
  } else if (IsAppWindow(window)) {
    TwelfWinPtr twin = (TwelfWinPtr)window;
    switch (twin->winType) {
      case TwelfWinDocument: {
        DocumentPtr doc = getDoc(window);
        selectAll = true;
        TEHandle te = doc->docTE;
        Boolean isSelection = (*te)->selStart < (*te)->selEnd;
        if (isSelection) copy = true;
        if (!isReadOnly(doc->docType)) {
          long offset;
          if (GetScrap(nil, 'TEXT', &offset) > 0) {
            paste = true;
          }
          if (isSelection) {
            cutClear = true;
          }
        }
      } break;
      case TwelfWinAbout: {
      } break;
    }
  }
  if (undo)
    EnableItem(menu, iUndo);
  else
    DisableItem(menu, iUndo);
  if (copy)
    EnableItem(menu, iCopy);
  else
    DisableItem(menu, iCopy);
  if (cutClear) {
    EnableItem(menu, iCut);
    EnableItem(menu, iClear);
  } else {
    DisableItem(menu, iCut);
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
}

void AdjustSignatureMenu() {
  WindowPtr window = FrontWindow();
  MenuHandle menu = GetMenuHandle(mSignature);

  Boolean enabled = false;

  if (IsAppWindow(window)) {
    TwelfWinPtr twin = (TwelfWinPtr)window;
    switch (twin->winType) {
      case TwelfWinDocument: {
        DocumentPtr doc = getDoc(window);
        if (!isReadOnly(doc->docType)) {
          enabled = true;
        }
      } break;
      case TwelfWinAbout: {
      } break;
    }
  }

  if (enabled) {
    EnableItem(menu, iEval);
    // EnableItem(menu, iEvalUnsafe);
  } else {
    DisableItem(menu, iEval);
    DisableItem(menu, iEvalUnsafe);
  }

  EnableItem(menu, iShowLog);
}

void AdjustMenus() {
  AdjustFileMenu();
  AdjustEditMenu();
  AdjustSignatureMenu();
}

void DoOpen() {
  StandardFileReply reply;
  SFTypeList types = {'TEXT'};

  StandardGetFile(nil, 1, types, &reply);
  if (!reply.sfGood) return;

  openFileSpec(&reply.sfFile);
}

/*	This is called when an item is chosen from the menu bar (after calling
        MenuSelect or MenuKey). It does the right thing for each command. */

// FIXME(safety): There may be some subtle assumptions about menu
// items having been enabled only if they're applicable to the front
// window. Probably should have getDoc do some more tag checking at runtime.
void DoMenuCommand(long menuResult) {
  short menuID, menuItem;
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
          ShowAboutBox();
          break;
        default: /* all non-About items in this menu are DAs et al */
          /* type Str255 is an array in MPW 3 */
          GetMenuItemText(GetMenuHandle(mApple), menuItem, daName);
          OpenDeskAcc(daName);
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
        case iClose: {
          WindowPtr window = FrontWindow();
          if (closeConfirmForWin(window)) {
            DoCloseWindow(window);
          }
        } break;
        case iSave: {
          DoSave(getDoc(FrontWindow()));
        } break;
        case iSaveAs: {
          DoSaveAs(getDoc(FrontWindow()));
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
        AdjustScrollbars(getDoc(window), false);  // FIXME(TE)
        AdjustTE(getDoc(window));                 // FIXME(TE)
      }
    } break;
    case mSignature: {
      switch (menuItem) {
        case iEval: {
          WindowPtr window = FrontWindow();
          DocumentPeek doc = getDoc(window);

          WindowPtr outWin = getOutputWindow();
          DocumentPeek outDoc = getDoc(outWin);
          TESetText("", 0, outDoc->docTE);
          setOutputDest(outDoc->docTE);

          CharsHandle textHandle = TEGetText(doc->docTE);
          TERec *tePtr = *(doc->docTE);

          logger("got the text: \"%.*s\"", tePtr->teLength, *textHandle);

          int len = tePtr->teLength;
          logger("about to allocate %d bytes...", len);
          char *buffer = (char *)allocate(len);
          logger("allocated");
          strncpy(buffer, (char *)*textHandle, len);
          logger("copied %d bytes to buffer", len);

          for (int i = 0; i < len; i++) {
            if (buffer[i] == '\r') {
              buffer[i] = '\n';
            }
          }
          logger("swizzled %d bytes from CR to NL...", len);

          int resp = execute();
          logger("Twelf response: %d", resp);

          // XXX raise an alert if abort?
          setOutputDest(NULL);
          char *abortStr = "%% ABORT %%";
          char *okStr = "%% OK %%";
          TEInsert(resp ? abortStr : okStr,
                   resp ? strlen(abortStr) : strlen(okStr), outDoc->docTE);
          // Scroll to insertion point
          TESelView(outDoc->docTE);
          // Update scrollbars as necessary
          AdjustScrollValues(outDoc, false);
          // I couldn't seem to get by with anything less than this, which
          // includes a whole EraseWindow. I tried just TEUpdate, I tried just
          // InvalRect, but when the output string grew shorter, it left
          // graphical cruft behind.
          DrawWindow(outWin);
        } break;
        case iEvalUnsafe: {
        } break;
        case iShowLog: {
          if (gLogWindow != NULL) {
            // This should really have been initialized already
            gLogWindow = getLogWindow();
          }
          ShowWindow(gLogWindow);
          SelectWindow(gLogWindow);
        }
      }
    } break;
  }
  HiliteMenu(0); /* unhighlight what MenuSelect (or MenuKey) hilited */
} /*DoMenuCommand*/
