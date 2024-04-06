/*------------------------------------------------------------------------------
#
#	Apple Macintosh Developer Technical Support
#
#	MultiFinder-Aware TextEdit Sample Application
#
#	TESample
#
#	TESample.r	-	Rez Source
#
#	Copyright © 1989 Apple Computer, Inc.
#	All rights reserved.
#
#	Versions:
#				1.00				08/88
#				1.01				11/88
#				1.02				04/89
#				1.03				06/89
#				1.04				06/92
#
#	Components:
#				TESample.p			June 1, 1989
#				TESample.c			June 1, 1989
#				TESampleInit.c		June 4, 1992
#				TESampleGlue.a		June 1, 1989
#				TESample.r			June 1, 1989
#				TESample.h			June 1, 1989
#				PTESample.make		June 1, 1989
#				CTESample.make		June 1, 1989
#				TCTESample.π		June 4, 1992
#				TCTESample.π.rsrc	June 4, 1992
#				TCTESampleGlue.c	June 4, 1992
#
#	TESample is an example application that demonstrates how
#	to initialize the commonly used toolbox managers, operate
#	successfully under MultiFinder, handle desk accessories and
#	create, grow, and zoom windows. The fundamental TextEdit
#	toolbox calls and TextEdit autoscroll are demonstrated. It
#	also shows how to create and maintain scrollbar controls.
#
#	It does not by any means demonstrate all the techniques you
#	need for a large application. In particular, Sample does not
#	cover exception handling, multiple windows/documents,
#	sophisticated memory management, printing, or undo. All of
#	these are vital parts of a normal full-sized application.
#
#	This application is an example of the form of a Macintosh
#	application; it is NOT a template. It is NOT intended to be
#	used as a foundation for the next world-class, best-selling,
#	600K application. A stick figure drawing of the human body may
#	be a good example of the form for a painting, but that does not
#	mean it should be used as the basis for the next Mona Lisa.
#
#	We recommend that you review this program or Sample before
#	beginning a new application. Sample is a simple app. which doesn’t
#	use TextEdit or the Control Manager.
#
------------------------------------------------------------------------------*/

#include "Types.r"
#include "resource-consts.h"

/* we use an MBAR resource to conveniently load all the menus */

resource 'MBAR' (rMenuBar, preload) {
	{ mApple, mFile, mEdit };		/* three menus */
};


resource 'MENU' (mApple, preload) {
	mApple, textMenuProc,
	0b1111111111111111111111111111101,	/* disable dashed line, enable About and DAs */
	enabled, apple,
	{
      "About Twelf\311", noIcon, noKey, noMark, plain,
		"-",
		noicon, nokey, nomark, plain
	}
};

resource 'MENU' (mFile, preload) {
	mFile, textMenuProc,
	0b0000000000000000000000000000000, /* enable nothing */
	enabled, "File",
	{
		"New",
			noicon, "N", nomark, plain;
		"Open",
			noicon, "O", nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Close",
			noicon, "W", nomark, plain;
		"Save",
			noicon, "S", nomark, plain;
		"Save As\311",
			noicon, nokey, nomark, plain;
		"Revert",
			noicon, nokey, nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Quit",
			noicon, "Q", nomark, plain
	}
};

resource 'MENU' (mEdit, preload) {
	mEdit, textMenuProc,
	0b0000000000000000000000000000000,	/* enable nothing */
	enabled, "Edit",
	 {
		"Undo",
			noicon, "Z", nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Cut",
			noicon, "X", nomark, plain;
		"Copy",
			noicon, "C", nomark, plain;
		"Paste",
			noicon, "V", nomark, plain;
		"Clear",
			noicon, nokey, nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Select All",
			noicon, "A", nomark, plain;
	}
};



/* this ALRT and DITL are used as an error screen */

resource 'ALRT' (rUserAlert, purgeable) {
	{40, 20, 160, 290},
	rUserAlert,
	{ /* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, silent
	},
    centerMainScreen       // Where to show the alert
};

resource 'DITL' (rUserAlert, purgeable) {
	{ /* array DITLarray: 3 elements */
		/* [1] */
		{80, 150, 100, 230},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{10, 60, 60, 230},
		StaticText {
			disabled,
			"Error. ^0."
		},
		/* [3] */
		{8, 8, 40, 40},
		Icon {
			disabled,
			2
		}
	}
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

// vertical scrollbar. initial rect is not particularly meaningful
resource 'CNTL' (rVScroll, preload, purgeable) {
	{-1, 385, 236, 401},
	0, visible, 0, 0, scrollBarProc, 0, ""
};

// horizontal scrollbar. initial rect is not particularly meaningful
resource 'CNTL' (rHScroll, preload, purgeable) {
	{235, -1, 251, 386},
	0, visible, 0, 0, scrollBarProc, 0, ""
};

// XXX are these used?
resource 'STR#' (kErrStrings, purgeable) {
	{
	"You must run on 512Ke or later";
	"Application Memory Size is too small";
	"Not enough memory to run";
	"Not enough memory to do Cut";
	"Cannot do Cut";
	"Cannot do Copy";
	"Cannot exceed 32,000 characters with Paste";
	"Not enough memory to do Paste";
	"Cannot create window";
	"Cannot exceed 32,000 characters";
	"Cannot do Paste"
	}
};

resource 'SIZE' (-1) {
   reserved,
	acceptSuspendResumeEvents,
   reserved,
	canBackground,				/* we can background; we don't currently, but our sleep value */
								/* guarantees we don't hog the Mac while we are in the background */
	multiFinderAware,			/* this says we do our own activate/deactivate; don't fake us out */
	backgroundAndForeground,	/* this is definitely not a background-only application! */
	dontGetFrontClicks,			/* change this is if you want "do first click" behavior like the Finder */
	ignoreChildDiedEvents,		/* essentially, I'm not a debugger (sub-launching) */
	not32BitCompatible,			/* this app should not be run in 32-bit address space */
   isHighLevelEventAware,
	reserved,
	reserved,
	reserved,
	reserved,
	reserved,
	reserved,
	kPrefSize * 1024,
	kMinSize * 1024
};


data 'TEXT' (rAboutText) {
    "About Twelf\r\r"
    "Twelf is a research project concerned with the design, implementation, "
    "and application of logical frameworks funded by the National Science "
    "Foundation under grants CCR-9619584 and CCR-9988281 Meta-Logical Frameworks, "
    "CCR-0306313 Efficient Logical Frameworks (Principal Investigator: Frank Pfenning) "
    "and by DARPA under the contract number F196268-95-C-0050 The Fox Project: Advanced "
    " Languages for Systems Software (Principal Investigators: Robert Harper, Peter "
	 "Lee, and Frank Pfenning)."
};

// Close confirm dialog. XXX should it be an ALRT instead?

resource 'DLOG' (rCloseConfirm) {
  {94, 80, 211, 434},
  dBoxProc,
  visible,
  noGoAway,
  0,
  rCloseConfirm,
  "",
  centerMainScreen,
};

resource 'DITL' (rCloseConfirm) {
	{
		{ 87, 284, 107, 344 }, Button { enabled, "Save" };
		{ 87, 211, 107, 271 }, Button { enabled, "Cancel" };
		{ 87, 70, 107, 155 }, Button { enabled, "Don't Save" };
		{ 10, 20, 42, 52 }, Icon { disabled, 2 };
		{ 10, 72, 76, 344 },	StaticText { enabled, "Save changes to the document \"^0\" before closing?" };
	}
};

// About box
resource 'WIND' (rAboutBoxWindow) {
    {0, 0, 220, 400}, altDBoxProc;
    invisible;
    noGoAway;
    0, "";
    noAutoCenter;
};

#include "icon.r"