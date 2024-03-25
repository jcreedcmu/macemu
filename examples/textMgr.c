/*
 * $Source: $
 * $Revision: $
 * $Date: $
 * $State: $
 * $Author: $
 *
 *
 * $Log: $
 * Revision 3.1  3/10/92 wade
 * Added Baylor's changes
 * #include "documentRec.h"
 *
 * Revision 3.2 6/15/92 mec
 * changed name of AdjustCursor to TextAdjustCursor
 *
 */
/*
 *	Copyright (c) 1991 by the Massachusetts Institute of Technology,
 *	For copying and distribution information, see the file
 *	"mit-copyright.h".
 *
 * 	textMgr.c  - WADE 8/23/89
 *		These routines are used only for the text edit window.  Any complicated routines were
 *		probably taken from the MPW 3.0 Examples folder.
 *
 *	text_bind
 *	text_clear
 *	text_draw
 *	AdjustCursor
 *	AdjustScrollbars
 *	AdjustScrollSizes
 *	AdjustViewRect
 *	VActionProc
 *	HActionProc
 *	text_get_buffer
 *	DoNew
 *	text_get_handle
 *	text_get_length
 *
 */

#include <Limits.h>
#include <Types.h>
#include <QuickDraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Controls.h>
#include <Windows.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Desk.h>
#include <Scrap.h>
#include <ToolUtils.h>
#include <Memory.h>
#include <SegLoad.h>
#include <Files.h>
#include <OSUtils.h>
#include <Traps.h>
#include <Lists.h>
#include <resources.h>
#include "pips.h"
#include "prototype.h"
#include "script.h"
#include "mit-copyright.h"
#include "documentRec.h"

extern Handle 		gTextStr;
extern Boolean		gInBackground;
extern WindowPtr	gTextWindow;
extern short		gNumDocuments;

/* Define TopLeft and BotRight macros for convenience. Notice the implicit
   dependency on the ordering of fields within a Rect */
#define TopLeft(aRect)	(* (Point *) &(aRect).top)
#define BotRight(aRect)	(* (Point *) &(aRect).bottom)

/* 	A reference to our assembly language routine that gets attached to the clikLoop
	field of our TE record. */
extern pascal void AsmClikLoop();

/*
 *
 * 	text_bind:
 *
 *	Add to the text buffer the string in the function parameter, be carefull not to exceed
 *	the size of the text buffer.  Return the number of bytes that could not be added to the
 *	text buffer.
 */
int
text_bind( newText )
char *newText;
{
	int				size;
	DocumentPeek 	doc;

	doc =  (DocumentPeek) gTextWindow;
	TEDeactivate(doc->docTE);

	/* don't exceed the 32k text size */
	size = text_get_length(gTextWindow);
	if (size + strlen(newText) <= TEXT_SIZE )
		TEInsert(newText,strlen(newText),doc->docTE);

	TEActivate(doc->docTE);
	return 0;
}

/*
 *
 * text_draw:
 *
 *
 */
text_draw()
{
	DocumentPeek 	doc;

	doc =  (DocumentPeek) gTextWindow;

	(*doc->docTE)->selEnd = 0;							/* move the insertion point to the top
														   of the document for use in word scan */
														/* make the view rect start at 0,0 */

	AdjustScrollbars( gTextWindow, false );				/* adjust scroll bars */

}

/*
 *
 * text_clear:
 *
 *
 */
text_clear()
{
	DocumentPeek 	doc;

	doc =  (DocumentPeek) gTextWindow;

	TEDeactivate(doc->docTE);
	TESetSelect(0,32767,doc->docTE);
	TEDelete(doc->docTE);
	TEActivate(doc->docTE);
}

/*
 *
 * 	text_get_buffer:
 *
 *	This routine places what is in the window text buffer into the gTextStr object.  This is
 *	needed as the user may type in the text window, therefore the display text (doc -> docTE) is
 *	not equal to that of the gTextStr object.
 */
text_get_buffer()
{
	TEPtr			te;

	te = *((DocumentPeek) gTextWindow)->docTE;

	strncpy(*gTextStr,*(te->hText),te->teLength);
	strncpy(&((*gTextStr)[te->teLength]),"\0",1);

}

/*
 *
 * 	GetTERect:
 *
 * 	Return a rectangle that is inset from the portRect by the size of
 *	the scrollbars and a little extra margin.
 */
#pragma segment Main
void GetTERect(window,teRect)
	WindowPtr	window;
	Rect		*teRect;
{
	*teRect = window->portRect;
	InsetRect(teRect, TEXT_MARGIN, TEXT_MARGIN);	/* adjust for margin */
	teRect->bottom = teRect->bottom - 15;			/* and for the scrollbars */
	teRect->right = teRect->right - 15;
} /*GetTERect*/

/*
 *	AdjustViewRect
 *
 * 	Update the TERec's view rect so that it is the greatest multiple of
 *	the lineHeight that still fits in the old viewRect.
 */
#pragma segment Main
void AdjustViewRect(docTE)
	TEHandle	docTE;
{
	TEPtr		te;

	te = *docTE;
	te->viewRect.bottom = (((te->viewRect.bottom - te->viewRect.top) / te->lineHeight)
							* te->lineHeight) + te->viewRect.top;
} /*AdjustViewRect*/

/*
 *
 * AdjustTE:
 *
 *
 * 	Scroll the TERec around to match up to the potentially updated scrollbar
 *	values. This is really useful when the window has been resized such that the
 *	scrollbars became inactive but the TERec was already scrolled.
 */
#pragma segment Main
void AdjustTE(window)
	WindowPtr	window;
{
	TEPtr		te;

	te = *((DocumentPeek)window)->docTE;
	TEScroll((te->viewRect.left - te->destRect.left) -
			GetCtlValue(((DocumentPeek)window)->docHScroll),
			(te->viewRect.top - te->destRect.top) -
				(GetCtlValue(((DocumentPeek)window)->docVScroll) *
				te->lineHeight),
			((DocumentPeek)window)->docTE);
} /*AdjustTE*/

/*
 *
 * AdjustHV:
 *
 *
 * 	Calculate the new control maximum value and current value, whether it is the horizontal or
 *	vertical scrollbar. The vertical max is calculated by comparing the number of lines to the
 *	vertical size of the viewRect. The horizontal max is calculated by comparing the maximum document
 *	width to the width of the viewRect. The current values are set by comparing the offset between
 *	the view and destination rects. If necessary and we canRedraw, have the control be re-drawn by
 *	calling ShowControl.
 */
#pragma segment Main
void AdjustHV(isVert,control,docTE,canRedraw)
	Boolean		isVert;
	ControlHandle control;
	TEHandle	docTE;
	Boolean		canRedraw;
{
	short		value, lines, max;
	short		oldValue, oldMax;
	TEPtr		te;

	oldValue = GetCtlValue(control);
	oldMax = GetCtlMax(control);
	te = *docTE;							/* point to TERec for convenience */
	if ( isVert ) {
		lines = te->nLines;
		/* since nLines isnÕt right if the last character is a return, check for that case */
		if ( *(*te->hText + te->teLength - 1) == kCrChar )
			lines += 1;
		max = lines - ((te->viewRect.bottom - te->viewRect.top) /
				te->lineHeight);
	} else
		max = (te->destRect.right - te->destRect.left) - (te->viewRect.right - te->viewRect.left);

	if ( max < 0 ) max = 0;
	SetCtlMax(control, max);

	/* Must deref. after SetCtlMax since, technically, it could draw and therefore move
		memory. This is why we donÕt just do it once at the beginning. */
	te = *docTE;
	if ( isVert )
		value = (te->viewRect.top - te->destRect.top) / te->lineHeight;
	else
		value = te->viewRect.left - te->destRect.left;

	if ( value < 0 ) value = 0;
	else if ( value >  max ) value = max;

	SetCtlValue(control, value);
	/* now redraw the control if it needs to be and can be */
	if ( canRedraw || (max != oldMax) || (value != oldValue) )
		ShowControl(control);
} /*AdjustHV*/

/*
 *
 * AdjustScrollValues:
 *
 * Simply call the common adjust routine for the vertical and horizontal scrollbars.
 */
#pragma segment Main
void AdjustScrollValues(window,canRedraw)
	WindowPtr	window;
	Boolean		canRedraw;
{
	DocumentPeek doc;

	doc = (DocumentPeek)window;
	AdjustHV(true, doc->docVScroll, doc->docTE, canRedraw);
	AdjustHV(false, doc->docHScroll, doc->docTE, canRedraw);
} /*AdjustScrollValues*/

/*	AdjustScrollSizes
 *
 *	Re-calculate the position and size of the viewRect and the scrollbars.
 *	kScrollTweek compensates for off-by-one requirements of the scrollbars
 *	to have borders coincide with the growbox.
 */
#pragma segment Main
void AdjustScrollSizes(window)
	WindowPtr	window;
{
	Rect		teRect;
	DocumentPeek doc;

	doc = (DocumentPeek) window;
	GetTERect(window, &teRect);							/* start with TERect */
	(*doc->docTE)->viewRect = teRect;
	AdjustViewRect(doc->docTE);							/* snap to nearest line */
	MoveControl(doc->docVScroll, window->portRect.right - kScrollbarAdjust, -1);
	SizeControl(doc->docVScroll, kScrollbarWidth, (window->portRect.bottom -
				window->portRect.top) - (kScrollbarAdjust - kScrollTweek));
	MoveControl(doc->docHScroll, -1, window->portRect.bottom - kScrollbarAdjust);
	SizeControl(doc->docHScroll, (window->portRect.right -
				window->portRect.left) - (kScrollbarAdjust - kScrollTweek),
				kScrollbarWidth);
} /*AdjustScrollSizes*/

/*
 * AdjustScrollbars
 *
 * Turn off the controls by jamming a zero into their contrlVis fields (HideControl erases them
 * and we don't want that). If the controls are to be resized as well, call the procedure to do that,
 * then call the procedure to adjust the maximum and current values. Finally re-enable the controls
 * by jamming a $FF in their contrlVis fields.
 */
#pragma segment Main
void AdjustScrollbars(window,needsResize)
	WindowPtr	window;
	Boolean		needsResize;
{
	DocumentPeek doc;

	doc = (DocumentPeek) window;
	/* First, turn visibility of scrollbars off so we wonÕt get unwanted redrawing */
	(*doc->docVScroll)->contrlVis = kControlInvisible;	/* turn them off */
	(*doc->docHScroll)->contrlVis = kControlInvisible;
	if ( needsResize )									/* move & size as needed */
		AdjustScrollSizes(window);
	AdjustScrollValues(window, needsResize);			/* fool with max and current value */
	/* Now, restore visibility in case we never had to ShowControl during adjustment */
	(*doc->docVScroll)->contrlVis = kControlVisible;	/* turn them on */
	(*doc->docHScroll)->contrlVis = kControlVisible;
} /* AdjustScrollbars */

/*
 *
 * PascalClickLoop:
 *
 * 	Gets called from our assembly language routine, AsmClikLoop, which is in
 *	turn called by the TEClick toolbox routine. Saves the windows clip region,
 *	sets it to the portRect, adjusts the scrollbar values to match the TE scroll
 *	amount, then restores the clip region.
 */
#pragma segment Main
pascal void PascalClikLoop()
{
	WindowPtr	window;
	RgnHandle	region;

	window = FrontWindow();
	region = NewRgn();
	GetClip(region);					/* save clip */
	ClipRect(&window->portRect);
	AdjustScrollValues(window, true);	/* pass true for canRedraw */
	SetClip(region);					/* restore clip */
	DisposeRgn(region);
} /* PascalClikLoop */

/*
 *
 * 	GetOldClickLoop:
 *
 * 	Gets called from our assembly language routine, AsmClikLoop, which is in
 *	turn called by the TEClick toolbox routine. It returns the address of the
 *	default clikLoop routine that was put into the TERec by TEAutoView to
 *	AsmClikLoop so that it can call it.
 */
#pragma segment Main
pascal ProcPtr GetOldClikLoop()
{
	return ((DocumentPeek)FrontWindow())->docClik;
} /* GetOldClikLoop */

/*
 *	AdjustCursor
 *
 *	Change the cursor's shape, depending on its position. This also calculates the region
 *	where the current cursor resides (for WaitNextEvent). When the mouse moves outside of
 *	this region, an event is generated. If there is more to the event than just
 *	Òthe mouse movedÓ, we get called before the event is processed to make sure
 *	the cursor is the right one. In any (ahem) event, this is called again before we
 *	fall back into WNE.
 */
#pragma segment Main
void TextAdjustCursor(mouse,region)
	Point		mouse;
	RgnHandle	region;
{
	WindowPtr	window;
	RgnHandle	arrowRgn;
	RgnHandle	iBeamRgn;
	Rect		iBeamRect;

	window = FrontWindow();	/* we only adjust the cursor when we are in front */

	/* let's not do this for the list window */
	if ( window != gTextWindow )
		return;

	if ( (! gInBackground) && (! IsDAWindow(window)) ) {
		/* calculate regions for different cursor shapes */
		arrowRgn = NewRgn();
		iBeamRgn = NewRgn();

		/* start arrowRgn wide open */
		SetRectRgn(arrowRgn, kExtremeNeg, kExtremeNeg, kExtremePos, kExtremePos);

		/* calculate iBeamRgn */
		if ( IsAppWindow(window) ) {
			iBeamRect = (*((DocumentPeek) window)->docTE)->viewRect;
			SetPort(window);	/* make a global version of the viewRect */
			LocalToGlobal(&TopLeft(iBeamRect));
			LocalToGlobal(&BotRight(iBeamRect));
			RectRgn(iBeamRgn, &iBeamRect);
			/* we temporarily change the portÕs origin to ÒglobalfyÓ the visRgn */
			SetOrigin(-window->portBits.bounds.left, -window->portBits.bounds.top);
			SectRgn(iBeamRgn, window->visRgn, iBeamRgn);
			SetOrigin(0, 0);
		}

		/* subtract other regions from arrowRgn */
		DiffRgn(arrowRgn, iBeamRgn, arrowRgn);

		/* change the cursor and the region parameter */
		if ( PtInRgn(mouse, iBeamRgn) ) {
			SetCursor(*GetCursor(iBeamCursor));
			CopyRgn(iBeamRgn, region);
		} else {
			SetCursor(&qd.arrow);
			CopyRgn(arrowRgn, region);
		}

		DisposeRgn(arrowRgn);
		DisposeRgn(iBeamRgn);
	}
} /*TextAdjustCursor*/

/*
 *	VActionProc:
 *
 * 	Determines how much to change the value of the vertical scrollbar by and how
 *	much to scroll the TE record.
 */
#pragma segment Main
pascal void VActionProc(control,part)
	ControlHandle control;
	short		part;
{
	short		amount;
	WindowPtr	window;
	TEPtr		te;

	if ( part != 0 ) {				/* if it was actually in the control */
		window = (*control)->contrlOwner;
		te = *((DocumentPeek) window)->docTE;
		switch ( part ) {
			case inUpButton:
			case inDownButton:		/* one line */
				amount = 1;
				break;
			case inPageUp:			/* one page */
			case inPageDown:
				amount = (te->viewRect.bottom - te->viewRect.top) / te->lineHeight;
				break;
		}
		if ( (part == inDownButton) || (part == inPageDown) )
			amount = -amount;		/* reverse direction for a downer */
		CommonAction(control, &amount);
		if ( amount != 0 )
			TEScroll(0, amount * te->lineHeight, ((DocumentPeek) window)->docTE);
	}
} /* VActionProc */

/*
 * 	HActionProc:
 *
 * 	Determines how much to change the value of the horizontal scrollbar by and how
 *	much to scroll the TE record.
 *
 */
#pragma segment Main
pascal void HActionProc(control,part)
	ControlHandle control;
	short		part;
{
	short		amount;
	WindowPtr	window;
	TEPtr		te;

	if ( part != 0 ) {
		window = (*control)->contrlOwner;
		te = *((DocumentPeek) window)->docTE;
		switch ( part ) {
			case inUpButton:
			case inDownButton:		/* a few pixels */
				amount = kButtonScroll;
				break;
			case inPageUp:			/* a page */
			case inPageDown:
				amount = te->viewRect.right - te->viewRect.left;
				break;
		}
		if ( (part == inDownButton) || (part == inPageDown) )
			amount = -amount;		/* reverse direction */
		CommonAction(control, &amount);
		if ( amount != 0 )
			TEScroll(amount, 0, ((DocumentPeek) window)->docTE);
	}
} /* HActionProc */

/*
 * DoNew:
 *
 * Create a new document and window.
 */
#pragma segment Main
void DoNew()
{
	Boolean		 good;
	Ptr			 storage;
	WindowPtr	 window;
	Rect		 destRect, viewRect;
	DocumentPeek doc;
	Rect		 tempRect;
	WStateData	*zoomRecord;
	Handle		 tempHandle;

	extern WindowPtr	gTextWindow;
	char				window_startup[6];

	storage = NewPtr(sizeof(DocumentRecord));
	if ( storage != nil ) {
		window = GetNewWindow(DOC_WINDOW, storage, (WindowPtr) -1);
		if ( window != nil ) {
			gTextWindow = window;		/* save this window pointer */
			gNumDocuments += 1;			/* this will be decremented when we call DoCloseWindow */
			SetPort(window);
			tempRect = qd.screenBits.bounds;

			/* subtract 42 (bottom margin 16, window banner 16, menu bar 10 */
			/* 501 is the width from the window resource for an 80 column window */
			/* 80 column window = 484 pix + 1 left border +  16 right scroll */
			/* 484 pix includes  2 indent for the left margin */

			pref_get_string(WINDOW_STARTUP, window_startup);
			if (strcmp("true",window_startup) || (!wind_check_bounds(window))) {
				/* use the default location and size */
				MoveWindow(window,DEFAULT_MENU_LEFT,DEFAULT_MENU_TOP + 20,false);

				/* subtract 82 , includes offset from the top, pull down menu, horizontal scroll bar */
				SizeWindow(window,501,tempRect.bottom - tempRect.top - 66,false);
			}

			doc =  (DocumentPeek) window;

			/* now, setup the max zoom window size */
			tempHandle = ((WindowPeek) window)->dataHandle;
			zoomRecord = (WStateData*)*tempHandle;
			zoomRecord -> stdState.right = zoomRecord -> stdState.left + 503; /* 1 + 484 + 16 */

			(((WindowPeek) gTextWindow) -> windowKind) = text;

			/* Set font, text size and style */
			TextSize(9);
			TextFont(monaco);

			GetTERect(window, &viewRect);
			destRect = viewRect;
			destRect.right = destRect.left + 501;  /* allow width to be 80 columns */
			doc->docTE = TENew(&destRect, &viewRect);

			AdjustViewRect(doc->docTE);
			TEAutoView(true, doc->docTE);
			doc->docClik = (ProcPtr) (*doc->docTE)->clikLoop;
			(*doc->docTE)->clikLoop = (ClikLoopProcPtr) AsmClikLoop;


			good = doc->docTE != nil;	/* if TENew succeeded, we have a good document */
			if ( good ) {				/* good document? Ñ get scrollbars */
				doc->docVScroll = GetNewControl(rVScroll, window);
				good = (doc->docVScroll != nil);
			}
			if ( good) {
				doc->docHScroll = GetNewControl(rHScroll, window);
				good = (doc->docHScroll != nil);
			}

			if ( good ) {				/* good? Ñ adjust & draw the controls, draw the window */
				/* false to AdjustScrollValues means musnÕt redraw; technically, of course,
				the window is hidden so it wouldnÕt matter whether we called ShowControl or not. */
				/* AdjustScrollValues(window, false); */
				ResizeWindow(window);
				/* ShowWindow(window); DON'T SHOW THE WINDOW ON STARTUP */

				/* now, create the text buffer, which will be binded to doc->docTE */
				gTextStr = NewHandle(TEXT_SIZE);
				**gTextStr == '/0';

			} else {
				DoCloseWindow(window,close_window);		/* otherwise regret we ever created it... */
				AlertUser( NO_WINDOW );		/* and tell user */
			}
		} else
			DisposPtr(storage);				/* get rid of the storage if it is never used */
	}
} /*DoNew*/

/*
 *
 * 	text_select_word:
 *
 *	Highlight selected area in text document.
 *
 */
void
text_select_word( start,end )
long	start;
long	end;
{
	DocumentPeek 	doc;
	short			line_end,line_start,scroll,ctr,view_lines,lines,max;
	Boolean			scroll_forward,last_page=false;

	doc =  (DocumentPeek) gTextWindow;

	line_start = ((*doc->docTE)->viewRect.top - (*doc->docTE)->destRect.top) /
				(*doc->docTE)->lineHeight;

	line_end =   line_start +
				 ((*doc->docTE)->viewRect.bottom - (*doc->docTE)->viewRect.top) /
				(*doc->docTE)->lineHeight;

	view_lines = line_end - line_start;

	lines = (*doc->docTE)->nLines;
	/* since nLines isnÕt right if the last character is a return, check for that case */
	if ( *((*doc->docTE)->hText + (*doc->docTE)->teLength - 1) == kCrChar )
		lines += 1;

	max = lines - ((*doc->docTE)->viewRect.bottom - (*doc->docTE)->viewRect.top) /
		(*doc->docTE)->lineHeight;

	/* is the selection range contained within the view rectangle? */
	scroll_forward = (start >= (*doc->docTE)->lineStarts[line_end]);

	/* is the selection range contained in the last page of the document?*/
	if (scroll_forward)
		last_page = (start >= (*doc->docTE)->lineStarts[max]);

	if (last_page) {
		/* goto the last page */
		SetCtlMax(doc->docVScroll,max);
		SetCtlValue(doc->docVScroll,max);
		AdjustTE(gTextWindow);
	}
	else
		if ( scroll_forward ) {
			/* selected string should be in the middle of the viewRect */
			scroll = view_lines * 0.5;

			/* find the number of lines to scroll, starting at the mid point of the viewRect */
			for (ctr=line_end;(*doc->docTE)->lineStarts[ctr]<start;ctr++,scroll++);

			/* scroll the TErect */
			TEScroll(0,(scroll * -(*doc->docTE)->lineHeight),doc->docTE);
		}

	TESetSelect(start,end,doc->docTE);
	AdjustScrollValues(gTextWindow,true);
}

/*
 *
 * 	text_select_end:
 *
 *	return end of selection range for the current window.
 *
 */
int
text_select_end()
{
	WindowPtr		window;
	DocumentPeek 	doc;

	window = FrontWindow();
	doc =  (DocumentPeek) window;

	return (*doc->docTE)->selEnd;
}

/*
 *
 * 	text_get_length:
 *
 *	return end size (length) of the text handle.
 *
 */
int
text_get_length(window)
WindowPtr	window;
{
	TEPtr			te;

	te = *((DocumentPeek) window)->docTE;

	return (te->teLength);
}

/*
 *
 * 	text_get_handle:
 *
 *	return the text handle.
 *
 */
Handle
text_get_handle(window)
WindowPtr	window;
{
	TEPtr			te;

	te = *((DocumentPeek) window)->docTE;

	return (te->hText);
}

/*
 *
 * text_soft_breaks () -- Turn "soft" line breaks into hard ones.
 *
 */
text_soft_breaks(window)
WindowPtr	window;
{
     short 		i,line_idx,save_sel_start,save_sel_end;
	 TEHandle	text_handle;

	 text_handle = ((DocumentPeek)window)->docTE;

     save_sel_start = (*text_handle) -> selStart;
     save_sel_end = (*text_handle) -> selEnd;

     for (i = 1; i < (*text_handle) -> nLines; i++) {
	   line_idx = (*text_handle) -> lineStarts[i];
	   if ((*(*text_handle) -> hText)[line_idx - 1] != '\n') {
	       TESetSelect(line_idx,line_idx,text_handle);
	       TEInsert("\n", 1, text_handle);
	  }
     }

     line_idx = (*text_handle) -> teLength;
	 if ((*(*text_handle) -> hText)[line_idx - 1] != '\n') {
     	TESetSelect(line_idx,line_idx,text_handle);
	  	TEInsert("\n", 1, text_handle);
     }

     TESetSelect(save_sel_start, save_sel_end,text_handle);
}

/*
 *
 * text_show_window
 *	 Make sure that the window is visible and that it is not obscured by another window
 *
 */
text_show_new_text(window)
WindowPtr	window;
{
	RgnHandle	tempRgn;

	text_clear();
	SelectWindow(window);

	if (!wind_visible(window))
		ShowWindow( window );

	/* eliminate redundant drawing */
	/* make the vis region equal to the update region */
	SetPort(gTextWindow);
	tempRgn = NewRgn();
	GetLocalUpdateRgn(window, tempRgn);
	ValidRgn(tempRgn);
	DisposeRgn(tempRgn);

}

/*
 *	CommonAction:
 *
 *	Common algorithm for pinning the value of a control. It returns the actual amount
 *	the value of the control changed. Note the pinning is done for the sake of returning
 *	the amount the control value changed.
 */
#pragma segment Main
void CommonAction(control,amount)
	ControlHandle control;
	short		*amount;
{
	short		value, max;

	value = GetCtlValue(control);	/* get current value */
	max = GetCtlMax(control);		/* and maximum value */
	*amount = value - *amount;
	if ( *amount < 0 )
		*amount = 0;
	else if ( *amount > max )
		*amount = max;
	SetCtlValue(control, *amount);
	*amount = value - *amount;		/* calculate the real change */
} /* CommonAction */

/*
 *
 * TECpyText:
 *
 * Fetch the text from the text handle and store as a pascal string
 * in Ptr p.  NOTE: string must be less than 255 characters.
 */
void
TECpyText(teH, p)
TEHandle	teH;
Ptr			p;
{
	p[0] = (unsigned char) (*teH)->teLength;
	BlockMove(*(*teH)->hText,&p[1],p[0]);
}

/*
 *
 * TELengthCheck(tePtr te, char c, int maxLen)
 *
 * Check that adding a character (c) to the text edit
 * te does not cause more than maxLen chars in the text
 * edit item:
 *
 * returns: false if ok, true if too large.
 *
 * this routine is from MacTutor.
 */
 int
 TELengthCheck(te,c,maxLen)
 TEPtr	te;
 char	c;
 int	maxLen;
 {
 	enum{CR=0x0d, DEL=0x7f, BS=0x08};

 	/* this char a del or bs, if so, does not increase */
	if (c == DEL || c == BS)
		return(false);

	/* selected a region? if so, does not increase */
	if (te->selStart < te->selEnd)
		return(false);

	/* else, will insert, check length */
	if (te->teLength < maxLen)
		return(false);

	return(true);
}

/*
 *
 * text_convert_tab
 *
 * This routine translates a tab into the appropriate number
 * of blanks, according to the tab stop position.
 */

text_convert_tab(te)
TEHandle	te;
 {
 	char	*sp;
	char	*cp;
	short	num_spaces,i;
	char    tab_stop_string[3];
	short	tab_stop;

  	pref_get_string(TAB_SETTING, tab_stop_string);
	tab_stop = atoi(tab_stop_string);

	/* find start of current line */
	sp = *(**te).hText;
	cp = sp + (**te).selStart-1;
	while (cp >= sp && *cp != '\n')
		cp--;

	num_spaces = tab_stop - ((sp + (**te).selStart - 1 - cp) % tab_stop);

	for (i = 0; i < num_spaces && ((*te)->teLength + 1 < kMaxTELength); i++)
		TEKey(' ', te);
 }

/*
 *   untabify () -- Convert tabs into spaces in a text handle.
 *
 */
untabify(tehandle)
TEHandle tehandle;
{
     char *cp,*ep;
     int idx;
     short num_spaces,pos,free_space;
     Handle htext;
	 Str255	blanks;
	 char   tab_stop_string[3];
	 short	tab_stop;

     htext = (**tehandle).hText;
     cp = *htext;
     ep = *htext + (**tehandle).teLength;
     free_space = kMaxTELength - (ep-cp);
	 TEDeactivate(tehandle);

     pos = 0;
	 memset(blanks,' ',255);
	 pref_get_string(TAB_SETTING, tab_stop_string);
	 tab_stop = atoi(tab_stop_string);
     for (; cp < ep; cp++) {
          if (*cp == '\n') {
               pos = 0;
          } else if (*cp == kTabChar) {
               num_spaces = tab_stop - (pos % tab_stop);
               idx = cp - *htext;
               TESetSelect(idx, idx+1, tehandle);
               TEDelete(tehandle);
               TEInsert("        ", num_spaces, tehandle);
               pos += num_spaces;
               free_space -= (num_spaces - 1);
               if (free_space < 0)
                    break;

               cp = *htext + idx + num_spaces - 1;      /* loop incrs cp */
               ep = *htext + (**tehandle).teLength;
          } else
               pos++;
     }

	 TEActivate(tehandle);
     return;
}
