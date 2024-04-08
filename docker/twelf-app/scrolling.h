#pragma once

#include <Windows.h>

#include "document.h"

void AdjustTE(DocumentPtr doc);
void AdjustHV(Boolean isVert, ControlHandle control, TEHandle docTE,
              Boolean canRedraw);
void AdjustScrollValues(DocumentPtr doc, Boolean canRedraw);
void AdjustScrollSizes(DocumentPtr doc);
void AdjustScrollbars(DocumentPtr doc, Boolean needsResize);

// Scrolling callbacks
pascal void VActionProc(ControlHandle control, short part);
pascal void HActionProc(ControlHandle control, short part);
