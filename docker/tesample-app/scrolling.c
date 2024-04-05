#include "scrolling.h"

#include <Windows.h>

#include "TESample.h"
#include "api.h"
#include "document.h"
#include "view-rects.h"

/* Scroll the TERec around to match up to the potentially updated scrollbar
        values. This is really useful when the window has been resized such that
   the scrollbars became inactive but the TERec was already scrolled. */

void AdjustTE(WindowPtr window) {
  TEPtr te;

  te = *((DocumentPeek)window)->docTE;
  TEScroll((te->viewRect.left - te->destRect.left) -
               GetControlValue(((DocumentPeek)window)->docHScroll),
           (te->viewRect.top - te->destRect.top) -
               (GetControlValue(((DocumentPeek)window)->docVScroll) *
                te->lineHeight),
           ((DocumentPeek)window)->docTE);
} /*AdjustTE*/

/* Calculate the new control maximum value and current value, whether it is the
   horizontal or vertical scrollbar. The vertical max is calculated by comparing
   the number of lines to the vertical size of the viewRect. The horizontal max
   is calculated by comparing the maximum document width to the width of the
   viewRect. The current values are set by comparing the offset between the view
   and destination rects. If necessary and we canRedraw, have the control be
   re-drawn by calling ShowControl. */

void AdjustHV(Boolean isVert, ControlHandle control, TEHandle docTE,
              Boolean canRedraw) {
  short value, lines, max;
  short oldValue, oldMax;
  TEPtr te;

  oldValue = GetControlValue(control);
  oldMax = GetControlMaximum(control);
  te = *docTE; /* point to TERec for convenience */
  if (isVert) {
    lines = te->nLines;
    /* since nLines isn't right if the last character is a return, check for
     * that case */
    if (*(*te->hText + te->teLength - 1) == kCrChar) lines += 1;
    max = lines - ((te->viewRect.bottom - te->viewRect.top) / te->lineHeight);
  } else
    max = kMaxDocWidth - (te->viewRect.right - te->viewRect.left);

  if (max < 0) max = 0;
  SetControlMaximum(control, max);

  /* Must deref. after SetControlMaximum since, technically, it could draw and
     therefore move memory. This is why we don't just do it once at the
     beginning. */
  te = *docTE;
  if (isVert)
    value = (te->viewRect.top - te->destRect.top) / te->lineHeight;
  else
    value = te->viewRect.left - te->destRect.left;

  if (value < 0)
    value = 0;
  else if (value > max)
    value = max;

  SetControlValue(control, value);
  /* now redraw the control if it needs to be and can be */
  if (canRedraw || (max != oldMax) || (value != oldValue)) ShowControl(control);
} /*AdjustHV*/

/* Simply call the common adjust routine for the vertical and horizontal
 * scrollbars. */

void AdjustScrollValues(WindowPtr window, Boolean canRedraw) {
  DocumentPeek doc;

  doc = (DocumentPeek)window;
  AdjustHV(true, doc->docVScroll, doc->docTE, canRedraw);
  AdjustHV(false, doc->docHScroll, doc->docTE, canRedraw);
} /*AdjustScrollValues*/

/*	Re-calculate the position and size of the viewRect and the scrollbars.
        kScrollTweek compensates for off-by-one requirements of the scrollbars
        to have borders coincide with the growbox. */

void AdjustScrollSizes(WindowPtr window) {
  Rect teRect;
  DocumentPeek doc;

  doc = (DocumentPeek)window;
  GetTERect(window, &teRect); /* start with TERect */
  (*doc->docTE)->viewRect = teRect;
  AdjustViewRect(doc->docTE); /* snap to nearest line */
  MoveControl(doc->docVScroll, window->portRect.right - kScrollbarAdjust, -1);
  SizeControl(doc->docVScroll, kScrollbarWidth,
              (window->portRect.bottom - window->portRect.top) -
                  (kScrollbarAdjust - kScrollTweek));
  MoveControl(doc->docHScroll, -1, window->portRect.bottom - kScrollbarAdjust);
  SizeControl(doc->docHScroll,
              (window->portRect.right - window->portRect.left) -
                  (kScrollbarAdjust - kScrollTweek),
              kScrollbarWidth);
} /*AdjustScrollSizes*/

/* Turn off the controls by jamming a zero into their contrlVis fields
   (HideControl erases them and we don't want that). If the controls are to be
   resized as well, call the procedure to do that, then call the procedure to
   adjust the maximum and current values. Finally re-enable the controls by
   jamming a $FF in their contrlVis fields. */

void AdjustScrollbars(WindowPtr window, Boolean needsResize) {
  DocumentPeek doc;

  doc = (DocumentPeek)window;
  /* First, turn visibility of scrollbars off so we won't get unwanted redrawing
   */
  (*doc->docVScroll)->contrlVis = kControlInvisible; /* turn them off */
  (*doc->docHScroll)->contrlVis = kControlInvisible;
  if (needsResize) /* move & size as needed */
    AdjustScrollSizes(window);
  AdjustScrollValues(window, needsResize); /* fool with max and current value */
  /* Now, restore visibility in case we never had to ShowControl during
   * adjustment */
  (*doc->docVScroll)->contrlVis = kControlVisible; /* turn them on */
  (*doc->docHScroll)->contrlVis = kControlVisible;
} /* AdjustScrollbars */
