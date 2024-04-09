#include "about.h"

#include "global-state.h"
#include "resource-consts.h"
#include "windows.h"

const int ORIG_PIC_WIDTH = 515;
const int ORIG_PIC_HEIGHT = 431;

const int PIC_WIDTH = ORIG_PIC_WIDTH * 0.35;
const int PIC_HEIGHT = ORIG_PIC_HEIGHT * 0.35;
const int WINDOW_HEIGHT = 290;
const int WINDOW_WIDTH = 300 + PIC_WIDTH;

AboutPtr mkAboutWindow(Rect *windowRect) {
  // We assume the input rect gives the appropriate size, but caller
  // is responsible for positioning and showing the window

  Ptr storage = NewPtr(sizeof(AboutRecord));
  if (storage == nil) return nil;

  Rect rfake;
  WindowPtr window = NewCWindow(storage, windowRect, "\pAbout Twelf\311", false,
                                altDBoxProc, (WindowPtr)-1, true, 0);

  if (window == nil) {
    DisposePtr(storage);
    return nil;
  }

  SetPort(window);  // I think this is necessary at this stage for associating
                    // the TE correctly

  AboutPtr adoc = (AboutPtr)window;

  adoc->winType = TwelfWinAbout;

  Handle txtHndl = GetResource('TEXT', rAboutText);   // FIXME(leak): dispose?
  Handle stylHndl = GetResource('styl', rAboutText);  // FIXME(leak): dispose?
  adoc->pic = GetPicture(rAboutPict);                 // FIXME(leak): dispose?
  adoc->bwPic = GetPicture(rAboutPict + 1);           // FIXME(leak): dispose?

  HLock(txtHndl);
  HLock(stylHndl);
  Rect r = *windowRect;
  r.left += PIC_WIDTH;
  InsetRect(&r, 10, 10);
  adoc->te = TEStyleNew(&r, &r);  // FIXME(leak): dispose?
  TEStyleInsert(*txtHndl, GetHandleSize(txtHndl), (StScrpHandle)stylHndl,
                adoc->te);
  ReleaseResource(stylHndl);
  ReleaseResource(txtHndl);

  return adoc;
}

void ShowAboutBox() {
  if (gAboutWindow != NULL) {
    SelectWindow(gAboutWindow);
    return;
  }

  Rect windowRect;
  SetRect(&windowRect, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

  AboutPtr adoc = mkAboutWindow(&windowRect);
  WindowPtr window = (WindowPtr)(&adoc->window);
  gAboutWindow = window;

  Rect picRect;
  SetRect(&picRect, 0, 0, PIC_WIDTH, PIC_HEIGHT);
  InsetRect(
      &picRect, 10,
      10);  // This doesn't exactly preserve aspect ratio, but close enough
  OffsetRect(&picRect, 0, (WINDOW_HEIGHT - PIC_HEIGHT) / 2);
  adoc->picRect = picRect;

  SizeWindow(window, WINDOW_WIDTH, WINDOW_HEIGHT, false);
  MoveWindow(
      window, qd.screenBits.bounds.right / 2 - window->portRect.right / 2,
      qd.screenBits.bounds.bottom / 2 - window->portRect.bottom / 2, false);
  ShowWindow(window);
}

void CloseAboutBox() {
  // FIXME(memleak): probably should dispose of more
  // things here
  CloseWindow(gAboutWindow);
  gAboutWindow = NULL;
}
