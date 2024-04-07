#include "menus.h"

#include <libtwelf.h>
#include <stdio.h>
#include <string.h>

#include "api.h"
#include "console.h"
#include "consts.h"
#include "dialogs.h"
#include "document.h"
#include "global-state.h"
#include "multiversal-stubs.h"
#include "resource-consts.h"
#include "save-ops.h"
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

  if (window != nil) {
    EnableItem(menu, iClose);
    EnableItem(menu, iSaveAs);
    DocumentPeek doc = getDoc(window);
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
}

void AdjustEditMenu() {
  WindowPtr window = FrontWindow();
  MenuHandle menu = GetMenuHandle(mEdit);

  Boolean undo = false;
  Boolean cutCopyClear = false;
  Boolean paste = false;
  Boolean selectAll = false;

  if (IsDAWindow(window)) {
    undo = true; /* all editing is enabled for DA windows */
    cutCopyClear = true;
    paste = true;
  } else if (IsAppWindow(window)) {
    selectAll = true;
    TEHandle te = getDoc(window)->docTE;
    if ((*te)->selStart < (*te)->selEnd) cutCopyClear = true;
    /* Cut, Copy, and Clear is enabled for app. windows with selections */
    long offset;
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
}

void AdjustSignatureMenu() {
  WindowPtr window = FrontWindow();
  MenuHandle menu = GetMenuHandle(mSignature);

  if (window != nil) {
    EnableItem(menu, iEval);
    // EnableItem(menu, iEvalUnsafe);
  } else {
    DisableItem(menu, iEval);
    DisableItem(menu, iEvalUnsafe);
  }
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

  WindowPtr window = mkDocumentWindow(TwelfDocument);
  if (window == NULL) return;
  DocumentPeek doc = (DocumentPeek)window;
  TEHandle te = doc->docTE;

  FSSpec *spec = &reply.sfFile;
  readFile(te, spec);

  associateFile(getDoc(window), spec);

  ShowWindow(window);
  InvalRect(&window->portRect);
}

void ShowAboutBox(void) {
  WindowPtr window;
  // should test for color availability
  window = GetNewCWindow(rAboutBoxWindow, NULL, (WindowPtr)-1);

  ((WindowPeek)window)->refCon = rAboutBoxWindow;
  MoveWindow(
      window, qd.screenBits.bounds.right / 2 - window->portRect.right / 2,
      qd.screenBits.bounds.bottom / 2 - window->portRect.bottom / 2, false);
  ShowWindow(window);
  SetPort(window);

  Handle h = GetResource('TEXT', rAboutText);
  HLock(h);
  Rect r = window->portRect;
  r.left += 260;
  InsetRect(&r, 10, 10);
  TETextBox(*h, GetHandleSize(h), &r, teJustLeft);
  ReleaseResource(h);

  PicHandle myPic = GetPicture(rAboutPict);

  Rect destRect;
  // original size 515x431
  SetRect(&destRect, 2, 2, 260, 218);
  DrawPicture(myPic, &destRect);

  while (!Button())
    ;
  while (Button())
    ;
  FlushEvents(everyEvent, 0);
  DisposeWindow(window);
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
          ShowAboutBox();
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
        case iClose: {
          WindowPtr window = FrontWindow();
          if (closeConfirmForDoc(getDoc(window))) {
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
        AdjustScrollbars(window, false);
        AdjustTE(window);
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

          unsigned char **textHandle = TEGetText(doc->docTE);
          TERec *tePtr = *(doc->docTE);

          printf("got the text: \"%.*s\"\r", tePtr->teLength, *textHandle);

          int len = tePtr->teLength;
          printf("about to allocate %d bytes...\r", len);
          char *buffer = (char *)allocate(len);
          printf("allocated\r");
          strncpy(buffer, *textHandle, len);
          printf("copied %d bytes to buffer\r", len);

          for (int i = 0; i < len; i++) {
            if (buffer[i] == '\r') {
              buffer[i] = '\n';
            }
          }
          printf("swizzled %d bytes from CR to NL...\r", len);

          int resp = execute();
          printf("Twelf response: %d\r", resp);

          // XXX raise an alert if abort?
          setOutputDest(NULL);
          char *abortStr = "%% ABORT %%";
          char *okStr = "%% OK %%";
          TEInsert(resp ? abortStr : okStr,
                   resp ? strlen(abortStr) : strlen(okStr), outDoc->docTE);
          // I couldn't seem to get by with anything less than this, which
          // includes a whole EraseWindow. I tried just TEUpdate, I tried just
          // InvalRect, but when the output string grew shorter, it left
          // graphical cruft behind.
          DrawWindow(outWin);
        } break;
        case iEvalUnsafe: {
        } break;
      }
    } break;
  }
  HiliteMenu(0); /* unhighlight what MenuSelect (or MenuKey) hilited */
} /*DoMenuCommand*/
