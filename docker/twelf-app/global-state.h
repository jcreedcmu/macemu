#pragma once

#include <Types.h>

/* The "g" prefix is used to emphasize that a variable is global. */

/* Are we still running? Set to false to cleanly ExitToShell */
extern Boolean gRunning;

/* Output window, if any */
extern WindowPtr gOutputWindow;

/* Log window, if any */
extern WindowPtr gLogWindow;

/* About window, if any */
extern WindowPtr gAboutWindow;

/* GMac is used to hold the result of a SysEnvirons call. This makes
   it convenient for any routine to check the environment. It is
   global information, anyway. */
extern SysEnvRec gMac; /* set up by Initialize */

/* GHasWaitNextEvent is set at startup, and tells whether the WaitNextEvent
   trap is available. If it is false, we know that we must call GetNextEvent. */
extern Boolean gHasWaitNextEvent; /* set up by Initialize */

/* GInBackground is maintained by our OSEvent handling routines. Any part of
   the program can check it to find out if it is currently in the background. */
extern Boolean gInBackground; /* maintained by Initialize and DoEvent */

/* GNumDocuments is used to keep track of how many open documents there are
   at any time. It is maintained by the routines that open and close documents.
 */
extern short
    gNumDocuments; /* maintained by Initialize, DoNew, and DoCloseWindow */

/* Keep track of whether twelf evaluation is currently running in the
 * background. */
typedef enum {
  TWELF_STATUS_NOT_RUNNING,
  TWELF_STATUS_RUNNING,
} TwelfStatus;

extern TwelfStatus gTwelfStatus;
