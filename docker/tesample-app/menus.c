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
