/*
  Copyright 2017 Wolfgang Thaller.

  This file is part of Retro68.

  Retro68 is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Retro68 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Retro68.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Quickdraw.h>
#include <Windows.h>
#include <Menus.h>
#include <Fonts.h>
#include <Resources.h>
#include <TextEdit.h>
#include <TextUtils.h>
#include <Dialogs.h>
#include <Devices.h>

static Rect initialWindowRect, nextWindowRect;

enum
  {
    kMenuApple = 128,
    kMenuFile,
    kMenuEdit
  };

enum
  {
    kItemAbout = 1,

    kItemNewDoc = 1,
    kItemNewRounded = 2,
    kItemClose = 3,
    kItemQuit = 5
  };

typedef struct {
  WindowRecord	docWindow;
  int          id;
  TEHandle		docTE;
} DocumentRecord, *DocumentPeek;

static int windowCounter = 0;

void MakeNewWindow(ConstStr255Param title, short procID)
{
  if(nextWindowRect.bottom > qd.screenBits.bounds.bottom
	  || nextWindowRect.right > qd.screenBits.bounds.right)
    {
		nextWindowRect = initialWindowRect;
    }

  DocumentRecord *storage = (DocumentRecord *)NewPtr(sizeof(DocumentRecord));
  storage->id = windowCounter++;
  WindowPtr w = NewWindow(storage, &nextWindowRect, title, true, procID, (WindowPtr) -1, true, 0);

  OffsetRect(&nextWindowRect, 15, 15);
}


void ShowAboutBox(void)
{
  WindowPtr w = GetNewWindow(128, NULL, (WindowPtr) - 1);
  MoveWindow(w,
				 qd.screenBits.bounds.right/2 - w->portRect.right/2,
				 qd.screenBits.bounds.bottom/2 - w->portRect.bottom/2,
				 false);
  ShowWindow(w);
  SetPort(w);

  Handle h = GetResource('TEXT', 128);
  HLock(h);
  Rect r = w->portRect;
  InsetRect(&r, 10,10);
  TETextBox(*h, GetHandleSize(h), &r, teJustLeft);

  ReleaseResource(h);
  while(!Button())
	 ;
  while(Button())
	 ;
  FlushEvents(everyEvent, 0);
  DisposeWindow(w);
}

void UpdateMenus(void)
{
  MenuRef m = GetMenu(kMenuFile);
  WindowPtr w = FrontWindow();
  if(w) // Close menu item: enabled if there is a window
	 EnableItem(m,kItemClose);
  else
	 DisableItem(m,kItemClose);

  m = GetMenu(kMenuEdit);
  if(w && GetWindowKind(w) < 0)
    {
		// Desk accessory in front: Enable edit menu items
		EnableItem(m,1);
		EnableItem(m,3);
		EnableItem(m,4);
		EnableItem(m,5);
		EnableItem(m,6);
    }
  else
    {
		// Application window or nothing in front, disable edit menu
		DisableItem(m,1);
		DisableItem(m,3);
		DisableItem(m,4);
		DisableItem(m,5);
		DisableItem(m,6);
    }

}

void DoMenuCommand(long menuCommand)
{
  Str255 str;
  WindowPtr w;
  short menuID = menuCommand >> 16;
  short menuItem = menuCommand & 0xFFFF;
  if(menuID == kMenuApple)
    {
		if(menuItem == kItemAbout)
		  ShowAboutBox();
		else
        {
			 GetMenuItemText(GetMenu(128), menuItem, str);
			 OpenDeskAcc(str);
        }
    }
  else if(menuID == kMenuFile)
    {
		switch(menuItem)
        {
		  case kItemNewDoc:
			 GetIndString(str,128,1);
			 MakeNewWindow(str, documentProc); // plain document window
			 break;
		  case kItemNewRounded:
			 GetIndString(str,128,2);
			 MakeNewWindow(str, 16); // rounded document window
			 break;

		  case kItemClose:    // close
			 w = FrontWindow();
			 if(w)
				{
				  if(GetWindowKind(w) < 0)
					 CloseDeskAcc(GetWindowKind(w));
				  else
					 DisposeWindow(FrontWindow());
				}
			 break;

		  case kItemQuit:
			 ExitToShell();
			 break;
        }
    }
  else if(menuID == kMenuEdit)
    {
		if(!SystemEdit(menuItem - 1))
        {
			 // edit command not handled by desk accessory
        }
    }

  HiliteMenu(0);
}

void DoUpdate(WindowPtr w)
{
  SetPort(w);
  BeginUpdate(w);

  int id = ((DocumentPeek)w)->id;

  Rect r;
  SetRect(&r, 20,20,120,120);
  FrameOval(&r);

  OffsetRect(&r, 32 * id, 32 * id);
  FillRoundRect(&r, 16, 16, &qd.ltGray);
  FrameRoundRect(&r, 16, 16);

  OffsetRect(&r, 32, 32);
  FillRect(&r, &qd.gray);
  FrameRect(&r);

  /* MoveTo(100,100); */
  /* DrawString("\pHello, world."); */

  EndUpdate(w);
}

int main(void)
{
  InitGraf(&qd.thePort);
  InitFonts();
  InitWindows();
  InitMenus();
  TEInit();
  InitDialogs(NULL);

  SetMenuBar(GetNewMBar(128));
  AppendResMenu(GetMenu(128), 'DRVR');
  DrawMenuBar();

  InitCursor();

  Rect r;
  SetRect(&initialWindowRect,20,60,400,260);
  nextWindowRect = initialWindowRect;

  for(;;)
    {
		EventRecord e;
		WindowPtr win;

		SystemTask();
		if(GetNextEvent(everyEvent, &e))
        {
			 switch(e.what)
            {
				case keyDown:
				  if(e.modifiers & cmdKey)
					 {
						UpdateMenus();
						DoMenuCommand(MenuKey(e.message & charCodeMask));
					 }
				  break;
				case mouseDown:
				  switch(FindWindow(e.where, &win))
					 {
					 case inGoAway:
						if(TrackGoAway(win, e.where))
						  DisposeWindow(win);
						break;
					 case inDrag:
						DragWindow(win, e.where, &qd.screenBits.bounds);
						break;
					 case inMenuBar:
						UpdateMenus();
						DoMenuCommand( MenuSelect(e.where) );
						break;
					 case inContent:
						SelectWindow(win);
						break;
					 case inSysWindow:
						SystemClick(&e, win);
						break;
					 }
				  break;
				case updateEvt:
				  DoUpdate((WindowPtr)e.message);
				  break;
            }
        }
    }
  return 0;
}
