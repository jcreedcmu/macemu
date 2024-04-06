#pragma once

#include <Types.h>

#include "api.h"

// FIXME: factor out scrolling code better
void AdjustScrollValues(WindowPtr window, Boolean canRedraw);

pascal void PascalClikLoop();
pascal TEClickLoopUPP GetOldClickLoop();
