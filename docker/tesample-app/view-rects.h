#pragma once

#include <TextEdit.h>
#include <Types.h>
#include <Windows.h>

void AdjustViewRect(TEHandle docTE);
void GetTERect(WindowPtr window, Rect *teRect);
