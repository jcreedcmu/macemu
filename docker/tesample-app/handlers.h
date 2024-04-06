#pragma once

#include <Events.h>
#include <Windows.h>

void GetGlobalMouse(Point *mouse);
void AdjustCursor(Point mouse, RgnHandle region);
unsigned long GetSleep(void);
void DoGrowWindow(WindowPtr window, EventRecord *event);
void DoZoomWindow(WindowPtr window, short part);
void _ResizeWindow(WindowPtr window);
void GetLocalUpdateRgn(WindowPtr window, RgnHandle localRgn);
void DoUpdate(WindowPtr window);
void DoDeactivate(WindowPtr window);
void DoActivate(WindowPtr window, Boolean becomingActive);
void DoContentClick(WindowPtr window, EventRecord *event);
void DoKeyDown(EventRecord *event);
void DoIdle(void);
void DoHighLevelEvent(EventRecord *event);
