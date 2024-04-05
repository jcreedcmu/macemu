/* A DocumentRecord contains the WindowRecord for one of our document windows,
   as well as the TEHandle for the text we are editing. Other document fields
   can be added to this record as needed. For a similar example, see how the
   Window Manager and Dialog Manager add fields after the GrafPort. */
typedef struct {
  WindowRecord docWindow;
  TEHandle docTE;
  ControlHandle docVScroll;
  ControlHandle docHScroll;
  TEClickLoopUPP docClick;
  Boolean fsSpecSet;
  FSSpec fsSpec;
} DocumentRecord, *DocumentPeek;
