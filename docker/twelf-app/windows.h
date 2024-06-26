#pragma once

#include <Windows.h>

#include "document.h"

WindowPtr getOutputWindow();
WindowPtr getLogWindow();
WindowPtr mkDocumentWindow(DocType docType);
AboutPtr mkAboutWindow();
void DoNew();
Boolean IsAppWindow(WindowPtr window);
Boolean IsDAWindow(WindowPtr window);
Boolean DoCloseWindow(WindowPtr window);
void DrawWindow(WindowPtr window);
void AlertUser(short error);
void Terminate();
