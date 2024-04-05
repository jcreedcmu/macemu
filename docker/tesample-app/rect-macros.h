#pragma once

/* Define TopLeft and BotRight macros for convenience. Notice the implicit
   dependency on the ordering of fields within a Rect */
#define TopLeft(aRect) (*(Point *)&(aRect).top)
#define BotRight(aRect) (*(Point *)&(aRect).bottom)
