#include "scrolling.h"

#include <Windows.h>

#include "api.h"
#include "consts.h"
#include "document.h"
#include "view-rects.h"

/* Scroll the TERec around to match up to the potentially updated scrollbar
        values. This is really useful when the window has been resized such that
   the scrollbars became inactive but the TERec was already scrolled. */

void AdjustTE(DocumentPtr doc) {
  TEPtr te = *doc->docTE;
  TEScroll((te->viewRect.left - te->destRect.left) -
               GetControlValue(doc->docHScroll),
           (te->viewRect.top - te->destRect.top) -
               (GetControlValue(doc->docVScroll) * te->lineHeight),
           doc->docTE);
}

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

void AdjustScrollValues(DocumentPtr doc, Boolean canRedraw) {
  AdjustHV(true, doc->docVScroll, doc->docTE, canRedraw);
  AdjustHV(false, doc->docHScroll, doc->docTE, canRedraw);
}

/*	Re-calculate the position and size of the viewRect and the scrollbars.
        kScrollTweek compensates for off-by-one requirements of the scrollbars
        to have borders coincide with the growbox. */

void AdjustScrollSizes(DocumentPtr doc) {
  Rect teRect;
  WindowPtr window = (WindowPtr)(&doc->window);

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
}

/* Turn off the controls by jamming a zero into their contrlVis fields
   (HideControl erases them and we don't want that). If the controls are to be
   resized as well, call the procedure to do that, then call the procedure to
   adjust the maximum and current values. Finally re-enable the controls by
   jamming a $FF in their contrlVis fields. */

void AdjustScrollbars(DocumentPtr doc, Boolean needsResize) {
  /* First, turn visibility of scrollbars off so we won't get unwanted redrawing
   */
  (*doc->docVScroll)->contrlVis = kControlInvisible; /* turn them off */
  (*doc->docHScroll)->contrlVis = kControlInvisible;
  if (needsResize) /* move & size as needed */
    AdjustScrollSizes(doc);
  AdjustScrollValues(doc, needsResize); /* fool with max and current value */
  /* Now, restore visibility in case we never had to ShowControl during
   * adjustment */
  (*doc->docVScroll)->contrlVis = kControlVisible; /* turn them on */
  (*doc->docHScroll)->contrlVis = kControlVisible;
} /* AdjustScrollbars */

/*	Common algorithm for pinning the value of a control. It returns the
   actual amount the value of the control changed. Note the pinning is done for
   the sake of returning the amount the control value changed. */

void CommonAction(ControlHandle control, short *amount) {
  short value, max;

  value = GetControlValue(control); /* get current value */
  max = GetControlMaximum(control); /* and maximum value */
  *amount = value - *amount;
  if (*amount < 0)
    *amount = 0;
  else if (*amount > max)
    *amount = max;
  SetControlValue(control, *amount);
  *amount = value - *amount; /* calculate the real change */
} /* CommonAction */

/* Determines how much to change the value of the vertical scrollbar by and how
        much to scroll the TE record. */

/* WARNING: Assumes the control came from a document window! */
pascal void VActionProc(ControlHandle control, short part) {
  short amount;
  WindowPtr window;
  TEPtr te;

  if (part != 0) { /* if it was actually in the control */
    window = (*control)->contrlOwner;
    te = *((DocumentPeek)window)->docTE;
    switch (part) {
      case kControlUpButtonPart:
      case kControlDownButtonPart: /* one line */
        amount = 1;
        break;
      case kControlPageUpPart: /* one page */
      case kControlPageDownPart:
        amount = (te->viewRect.bottom - te->viewRect.top) / te->lineHeight;
        break;
    }
    if ((part == kControlDownButtonPart) || (part == kControlPageDownPart))
      amount = -amount; /* reverse direction for a downer */
    CommonAction(control, &amount);
    if (amount != 0)
      TEScroll(0, amount * te->lineHeight, ((DocumentPeek)window)->docTE);
  }
}

/* Determines how much to change the value of the horizontal scrollbar by and
how much to scroll the TE record. */

/* WARNING: Assumes the control came from a document window! */
pascal void HActionProc(ControlHandle control, short part) {
  short amount;
  WindowPtr window;
  TEPtr te;

  if (part != 0) {
    window = (*control)->contrlOwner;
    te = *((DocumentPeek)window)->docTE;
    switch (part) {
      case kControlUpButtonPart:
      case kControlDownButtonPart: /* a few pixels */
        amount = kButtonScroll;
        break;
      case kControlPageUpPart: /* a page */
      case kControlPageDownPart:
        amount = te->viewRect.right - te->viewRect.left;
        break;
    }
    if ((part == kControlDownButtonPart) || (part == kControlPageDownPart))
      amount = -amount; /* reverse direction */
    CommonAction(control, &amount);
    if (amount != 0) TEScroll(amount, 0, ((DocumentPeek)window)->docTE);
  }
}
