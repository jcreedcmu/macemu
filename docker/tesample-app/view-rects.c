#include "view-rects.h"

#include "consts.h"

/* Return a rectangle that is inset from the portRect by the size of
        the scrollbars and a little extra margin. */

void GetTERect(WindowPtr window, Rect *teRect) {
  *teRect = window->portRect;
  InsetRect(teRect, kTextMargin, kTextMargin); /* adjust for margin */
  teRect->bottom = teRect->bottom - 15;        /* and for the scrollbars */
  teRect->right = teRect->right - 15;
} /*GetTERect*/

/* Update the TERec's view rect so that it is the greatest multiple of
        the lineHeight that still fits in the old viewRect. */

void AdjustViewRect(TEHandle docTE) {
  TEPtr te;

  te = *docTE;
  te->viewRect.bottom =
      (((te->viewRect.bottom - te->viewRect.top) / te->lineHeight) *
       te->lineHeight) +
      te->viewRect.top;
} /*AdjustViewRect*/
