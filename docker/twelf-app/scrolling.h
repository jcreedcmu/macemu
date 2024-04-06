#pragma once

#include <Windows.h>

void AdjustTE(WindowPtr window);
void AdjustHV(Boolean isVert, ControlHandle control, TEHandle docTE,
              Boolean canRedraw);
void AdjustScrollValues(WindowPtr window, Boolean canRedraw);
void AdjustScrollSizes(WindowPtr window);
void AdjustScrollbars(WindowPtr window, Boolean needsResize);

// Scrolling callbacks
pascal void VActionProc(ControlHandle control, short part);
pascal void HActionProc(ControlHandle control, short part);
