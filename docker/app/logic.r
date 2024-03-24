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

resource 'MENU' (128) {
    128, textMenuProc;
    allEnabled, enabled;
    apple;
    {
        "About WDEF Shell...", noIcon, noKey, noMark, plain;
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
        "Standard Document Window",
        "Rounded Document Window",
    }
};

data 'TEXT' (128) {
    "About WDEF Shell\r\r"
    "A Sample program that tries to show both how to write a basic classic Mac application, "
    "and how to write a custom window definition procedure (WDEF) using Retro68.\r"
    "Other code resources (CDEF, MDEF, LDEF, ...) aren't that much different.\r"
    "\r\rWritten in 2018, so everything here has been obsolete for two decades."
};

resource 'WIND' (128) {
    {0, 0, 220, 320}, altDBoxProc;
    invisible;
    noGoAway;
    0, "";
    noAutoCenter;
};

resource 'ICN#' (128) {
  {
  	 $"FFFF FFFF 0000 0000 FFFF FFFF 0000 0000", // actual icon
    $"EEEE EEEE EEEE EEEE EEEE EEEE EEEE EEEE"  // mask
  }
};

resource 'icl8' (128) {
  	 $"FFFF FFFF 0000 0000 FFFF FFFF 0000 0000"
};

resource 'BNDL' (128) {
  'TWLF', 0;
  {
    'FREF', { 0, 128 };
    'ICN#', { 0, 128 };
  }
};

resource 'FREF' (128) {
  'APPL', 0, "";
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
