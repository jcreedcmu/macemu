/*
 * This file is included by both the .r file and by .c files. This
 * means we should only put #defines in here, not other
 * C-language-specific material.
 */

/* The menu IDs have an "m" prefix and the item numbers within each
   menu have an "i" prefix. */
#define mApple 128 /* Apple menu */
#define iAbout 1

#define mFile 129 /* File menu */
#define iNew 1
#define iOpen 2
#define iClose 4
#define iSave 5
#define iSaveAs 6
#define iRevert 7
#define iQuit 9

#define mEdit 130 /* Edit menu */
#define iUndo 1
#define iCut 3
#define iCopy 4
#define iPaste 5
#define iClear 6
#define iSelectAll 8

#define mSignature 131 /* Signature menu */
#define iEval 1
#define iEvalUnsafe 2

#define rMenuBar 128      /* application's menu bar */
#define rCloseConfirm 128 /* close-confirm dialog box */
#define rUserAlert 129    /* user error alert */
#define rUserMessage 130  /* user message */
#define rDocWindow 128    /* application's window */
#define rVScroll 128      /* vertical scrollbar control */
#define rHScroll 129      /* horizontal scrollbar control */
#define kErrStrings 128   /* error string list */

#define kPrefSize 16 * 1024
#define kMinSize 8 * 1024

#define rAboutText 128
#define rAboutPict 128
