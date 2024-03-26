/*
	Copyright 2017 Wolfgang Thaller.

	This file is part of Retro68.

	Retro68 is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Retro68 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Retro68.  If not, see <http://www.gnu.org/licenses/>.
*/

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
        "New Document Window", noIcon, "N", noMark, plain;
        "New Rounded Window", noIcon, "M", noMark, plain;
        "Close", noIcon, "W", noMark, plain;
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
        "Rounded Document Window",
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
#ifdef TARGET_API_MAC_CARBON
	500 * 1024,	// Carbon apparently needs additional memory.
	500 * 1024
#else
	100 * 1024,
	100 * 1024
#endif
};

type 'CNTL' {
  rect;
  integer; // value
  byte invisible, visible;
  fill byte;
  integer; // max
  integer; // min
  integer scrollBarProc = 16; // procID
  unsigned longint;   // refCon
  pstring;
};

resource 'CNTL' (rVScroll, preload, purgeable) {
	{-1, 385, 236, 401},
	0, visible, 0, 0, scrollBarProc, 0, ""
};

#include "icon.r"