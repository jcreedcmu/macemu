#include "Processes.r"
#include "Menus.r"
#include "Windows.r"
#include "MacTypes.r"
#include "twelf.h"

resource 'MENU' (128) {
    128, textMenuProc;
    allEnabled, enabled;
    apple;
    {
        "About Twelf...", noIcon, noKey, noMark, plain;
        "-", noIcon, noKey, noMark, plain;
    }
};

resource 'MENU' (129) {
    129, textMenuProc;
    allEnabled, enabled;
    "File";
    {
        "New", noIcon, "N", noMark, plain;
        "-", noIcon, noKey, noMark, plain;
        "Quit", noIcon, "Q", noMark, plain;
    }
};

resource 'MENU' (130) {
    130, textMenuProc;
    0, enabled;
    "Edit";
    {
        "Undo", noIcon, "Z", noMark, plain;
        "-", noIcon, noKey, noMark, plain;
        "Cut", noIcon, "X", noMark, plain;
        "Copy", noIcon, "C", noMark, plain;
        "Paste", noIcon, "V", noMark, plain;
        "Clear", noIcon, noKey, noMark, plain;
    }
};

resource 'MBAR' (128) {
    { 128, 129, 130 };
};

resource 'STR#' (128) {
    {
        "Twelf",
    }
};

data 'TEXT' (128) {
    "About Twelf\r\r"
    "Twelf is a research project concerned with the design, implementation, "
    "and application of logical frameworks funded by the National Science "
    "Foundation under grants CCR-9619584 and CCR-9988281 Meta-Logical Frameworks, "
    "CCR-0306313 Efficient Logical Frameworks (Principal Investigator: Frank Pfenning) "
    "and by DARPA under the contract number F196268-95-C-0050 The Fox Project: Advanced "
    " Languages for Systems Software (Principal Investigators: Robert Harper, Peter "
	 "Lee, and Frank Pfenning)."
};

resource 'WIND' (128) {
    {0, 0, 220, 400}, altDBoxProc;
    invisible;
    noGoAway;
    0, "";
    noAutoCenter;
};

resource 'SIZE' (-1) {
    reserved,
    acceptSuspendResumeEvents,
    reserved,
    canBackground,
    doesActivateOnFGSwitch,
    backgroundAndForeground,
    dontGetFrontClicks,
    ignoreChildDiedEvents,
    is32BitCompatible,
    isHighLevelEventAware,
    onlyLocalHLEvents,
    notStationeryAware,
    dontUseTextEditServices,
    reserved,
    reserved,
    reserved,
    90 * 1024 * 1024,
    32 * 1024 * 1024
};

type 'CNTL' {
  rect;
  integer; // value
  byte invisible, visible;
  fill byte;
  integer; // max
  integer; // min
  integer buttonProc = 0, checkBoxProc = 1, radioButtonProc = 2, scrollBarProc = 16; // procID
  unsigned longint;   // refCon
  pstring;
};

/* Vertical scrollbar */
resource 'CNTL' (rVScroll, preload, purgeable) {
	{-1, 385, 236, 401},
	0, visible, 0, 0, scrollBarProc, 0, ""
};

/* Execute button */
resource 'CNTL' (rExecButton, preload, purgeable) {
	{kInputOffY + kInputHeight + kMargin, kInputOffX,
	kInputOffY + kInputHeight + kMargin + kButtonHeight, kInputOffX + kButtonWidth},
	0, visible, 0, 0, buttonProc, 0, "Check"
};

/* Debug checkbox */
resource 'CNTL' (rDebugCheckbox, preload, purgeable) {
	{kInputOffY + kInputHeight + kMargin, kInputOffX + kButtonWidth + kMargin,
	kInputOffY + kInputHeight + kMargin + kButtonHeight, kInputOffX + kButtonWidth + kButtonWidth + kMargin},
	0, visible, 0, 0, checkBoxProc, 0, "Debug"
};

#include "icon.r"