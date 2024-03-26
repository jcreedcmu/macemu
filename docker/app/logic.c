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
#include <string.h>
#include <stdio.h>
#include "logic.h"

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
  WindowRecord	 docWindow;
  int           id;
  TEHandle		 docTE;
  ControlHandle docVScroll;
} DocumentRecord, *DocumentPeek;

static int windowCounter = 0;

void GetTERect(WindowPtr window, Rect *teRect) {
	*teRect = window->portRect;
	InsetRect(teRect, 2, 2);	/* adjust for margin */
	teRect->bottom = teRect->bottom - 15;		/* and for the scrollbars */
	teRect->right = teRect->right - 15;
}

void MakeNewWindow(ConstStr255Param title, short procID)
{
  if (nextWindowRect.bottom > qd.screenBits.bounds.bottom
	  || nextWindowRect.right > qd.screenBits.bounds.right)
    {
		nextWindowRect = initialWindowRect;
    }

  int id = windowCounter++;
  Ptr storage = NewPtr(sizeof(DocumentRecord));
  WindowPtr w = NewWindow(storage, &nextWindowRect, title, true, procID, (WindowPtr) -1, true, id);
  DocumentPeek doc = (DocumentPeek) w;

  Rect destRect, viewRect;
  //  SetRect(&destRect, 0,0,400-MARGIN,260-MARGIN);
  // SetRect(&viewRect, 0,0,400-MARGIN,260-MARGIN);
  GetTERect(w, &destRect);
  GetTERect(w, &viewRect);

  SetPort(w);
  doc->docTE = TENew(&destRect, &viewRect);
  TEActivate(doc->docTE);
  if (doc->docTE) {
	 printf("Ok, we have a docTE\r");
  }
  else {
	 printf("Oops, NULL docTE\r");
  }
  // TESetText("abcdefghij", 10, doc->docTE);
  doc->id = id;

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
  if (w) // Close menu item: enabled if there is a window
	 EnableItem(m,kItemClose);
  else
	 DisableItem(m,kItemClose);

  m = GetMenu(kMenuEdit);
  if (w && GetWindowKind(w) < 0)
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
  if (menuID == kMenuApple)
    {
		if (menuItem == kItemAbout)
		  ShowAboutBox();
		else
        {
			 GetMenuItemText(GetMenu(128), menuItem, str);
			 OpenDeskAcc(str);
        }
    }
  else if (menuID == kMenuFile)
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
			 if (w)
				{
				  if (GetWindowKind(w) < 0)
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
  else if (menuID == kMenuEdit)
    {
		if (!SystemEdit(menuItem - 1))
        {
			 // edit command not handled by desk accessory
        }
    }

  HiliteMenu(0);
}


void DrawWindow (WindowPtr w) {
  SetPort(w);

  DocumentPeek doc = (DocumentPeek)w;


  /* OffsetRect(&r, 32 * id, 32 * id); */
  /* FillRoundRect(&r, 16, 16, &qd.ltGray); */
  /* FrameRoundRect(&r, 16, 16); */

  /* OffsetRect(&r, 32, 32); */
  /* FillRect(&r, &qd.gray); */
  /* FrameRect(&r); */


  EraseRect(&w->portRect);
  int id = doc->id;

  /* char buf[256]; */
  /* sprintf(buf+1, "-- %d --", id); */
  /* buf[0] = strlen(buf+1); */
  /* MoveTo(10,10); */
  /* DrawString(buf); */

  TEUpdate(&w->portRect, doc->docTE);
  /* Rect r; */
  /* SetRect(&r, 0,0,120,120); */
  /* OffsetRect(&r, 32 * id, 32 * id); */
  /* FrameOval(&r); */
}


Boolean IsAppWindow(WindowPtr window) {
  short		windowKind;

  if (window == 0)
	 return false;
  else {	/* application windows have windowKinds = userKind (8) */
	 windowKind = ((WindowPeek) window)->windowKind;
	 return (windowKind == userKind);
  }
}


void DoUpdate(WindowPtr window) {
  if (IsAppWindow(window)) {
	 BeginUpdate(window);				/* this sets up the visRgn */
	 if (! EmptyRgn(window->visRgn) )	/* draw if updating needs to be done */
		DrawWindow(window);
	 EndUpdate(window);
  }
}

void DoIdle(void) {
  WindowPtr window;
  window = FrontWindow();
  if (IsAppWindow(window) )
	 TEIdle(((DocumentPeek) window)->docTE);
}


void AdjustTE(WindowPtr	window) {
  /* TEPtr		te; */

  /* te = *((DocumentPeek)window)->docTE; */
  /* TEScroll((te->viewRect.left - te->destRect.left) - */
  /* 		GetControlValue(((DocumentPeek)window)->docHScroll), */
  /* 		(te->viewRect.top - te->destRect.top) - */
  /* 			(GetControlValue(((DocumentPeek)window)->docVScroll) * */
  /* 			te->lineHeight), */
  /* 		((DocumentPeek)window)->docTE); */
}

void DoKeyDown(EventRecord *event) {
  WindowPtr	window;
  char		key;
  TEHandle	te;

  window = FrontWindow();
  if (IsAppWindow(window)) {
	 te = ((DocumentPeek) window)->docTE;
	 key = event->message & charCodeMask;
	 /* we have a char. for our window; see if we are still below TextEdit's
		 limit for the number of characters (but deletes are always rad) */
	 if (key == kDelChar ||
		  (*te)->teLength - ((*te)->selEnd - (*te)->selStart) + 1 <
		  kMaxTELength) {
		TEKey(key, te);
		// AdjustScrollbars(window, false);
		AdjustTE(window);
	 } else {
		//		AlertUser(eExceedChar);
	 }
  }
}


void GetLocalUpdateRgn(WindowPtr window, RgnHandle localRgn) {
  CopyRgn(((WindowPeek) window)->updateRgn, localRgn);	/* save old update region */
  OffsetRgn(localRgn, window->portBits.bounds.left, window->portBits.bounds.top);
}


void DoActivate(WindowPtr window, Boolean becomingActive) {
  RgnHandle	tempRgn, clipRgn;
  Rect      growRect;
  DocumentPeek doc;

	if (IsAppWindow(window)) {
	  doc = (DocumentPeek) window;
	  if (becomingActive) {
		 /*	since we don't want TEActivate to draw a selection in an area where
				we're going to erase and redraw, we'll clip out the update region
				before calling it. */
		 tempRgn = NewRgn();
		 clipRgn = NewRgn();
		 GetLocalUpdateRgn(window, tempRgn);			/* get localized update region */
		 GetClip(clipRgn);
		 DiffRgn(clipRgn, tempRgn, tempRgn);			/* subtract updateRgn from clipRgn */
		 SetClip(tempRgn);
		 TEActivate(doc->docTE);
		 SetClip(clipRgn);							/* restore the full-blown clipRgn */
		 DisposeRgn(tempRgn);
		 DisposeRgn(clipRgn);

		 /* the controls must be redrawn on activation: */
		 (*doc->docVScroll)->contrlVis = kControlVisible;

		 InvalRect(&(*doc->docVScroll)->contrlRect);

		 /* the growbox needs to be redrawn on activation: */
		 growRect = window->portRect;
		 /* adjust for the scrollbars */
		 growRect.top = growRect.bottom - kScrollbarAdjust;
		 growRect.left = growRect.right - kScrollbarAdjust;
		 InvalRect(&growRect);
	  }
	  else {
		 TEDeactivate(doc->docTE);
		 /* the controls must be hidden on deactivation: */
		 HideControl(doc->docVScroll);
			 /* the growbox should be changed immediately on deactivation: */
		 DrawGrowIcon(window);
	  }
	}
}

void DoContentClick(WindowPtr window, EventRecord *event) {
  Point		    mouse;
  ControlHandle control;
  short		    part, value;
  Boolean		 shiftDown;
  DocumentPeek  doc;
  Rect		    teRect;

	if (IsAppWindow(window)) {
		SetPort(window);
		mouse = event->where;							/* get the click position */
		GlobalToLocal(&mouse);
		doc = (DocumentPeek) window;
		/* see if we are in the viewRect. if so, we won't check the controls */
		GetTERect(window, &teRect);
		if (PtInRect(mouse, &teRect)) {
			/* see if we need to extend the selection */
			shiftDown = (event->modifiers & shiftKey) != 0;	/* extend if Shift is down */
			TEClick(mouse, shiftDown, doc->docTE);
		} else {
			/* part = FindControl(mouse, window, &control); */
			/* switch (part) { */
			/* 	case 0:							/\* do nothing for viewRect case *\/ */
			/* 		break; */
			/* 	case kControlIndicatorPart: */
			/* 		value = GetControlValue(control); */
			/* 		part = TrackControl(control, mouse, nil); */
			/* 		if (part != 0) { */
			/* 			value -= GetControlValue(control); */
			/* 			/\* value now has CHANGE in value; if value changed, scroll *\/ */
			/* 			if (value != 0 ) */
			/* 				if (control == doc->docVScroll ) */
			/* 					TEScroll(0, value * (*doc->docTE)->lineHeight, doc->docTE); */
			/* 				else */
			/* 					TEScroll(value, 0, doc->docTE); */
			/* 		} */
			/* 		break; */
			/* 	default:						/\* they clicked in an arrow, so track & scroll *\/ */
			/* 		if (control == doc->docVScroll ) */
			/* 			value = TrackControl(control, mouse, (ControlActionUPP) VActionProc); */
			/* 		else */
			/* 			value = TrackControl(control, mouse, (ControlActionUPP) HActionProc); */
			/* 		break; */
			/* } */
		}
	}
}

int main(void) {
  stdout = fopen("out", "w");
  setbuf(stdout, NULL);

  printf("Hello, World\r");

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

  for(;;) {
	 EventRecord e;
	 WindowPtr win;

	 SystemTask();
	 if (GetNextEvent(everyEvent, &e)) {
		switch(e.what) {
		case keyDown: // intentional fallthrough
		case autoKey:
		  if (e.modifiers & cmdKey) {
			 UpdateMenus();
			 DoMenuCommand(MenuKey(e.message & charCodeMask));
		  }
		  else {
			 DoKeyDown(&e);
		  }
		  break;
		case mouseDown:
		  switch(FindWindow(e.where, &win)) {
		  case inGoAway:
			 if (TrackGoAway(win, e.where))
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
			 if (win != FrontWindow()) {
				SelectWindow(win);
				/*DoEvent(event);*/	/* uncomment this line for "do first click" */
			 } else
				DoContentClick(win, &e);
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
	 else {
		DoIdle();
	 }
  }
  return 0;
}
