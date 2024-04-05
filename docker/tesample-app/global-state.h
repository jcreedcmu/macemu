#pragma once

#include <Types.h>

/* The "g" prefix is used to emphasize that a variable is global. */

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
