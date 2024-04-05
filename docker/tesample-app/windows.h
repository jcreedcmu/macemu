#pragma once

#include <Windows.h>

void DoNew(void);
Boolean IsAppWindow(WindowPtr window);
Boolean IsDAWindow(WindowPtr window);
Boolean DoCloseWindow(WindowPtr window);
void DrawWindow(WindowPtr window);
void AlertUser(short error);
