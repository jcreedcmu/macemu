#include "global-state.h"

Boolean gRunning;
WindowPtr gOutputWindow = NULL;
WindowPtr gAboutWindow = NULL;
WindowPtr gLogWindow = NULL;
SysEnvRec gMac;
Boolean gHasWaitNextEvent;
Boolean gInBackground;
short gNumDocuments;
