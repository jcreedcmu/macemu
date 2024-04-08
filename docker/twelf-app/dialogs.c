#include "dialogs.h"

#include "file-ops.h"

short closeConfirm(FSSpec *spec) {
  DialogPtr dlg = GetNewDialog(128, nil, (WindowPtr)-1);
  DialogItemType type;
  Handle itemH;
  Rect box;

  GetDialogItem(dlg, rCloseConfirm_SaveButtonIndex, &type, &itemH, &box);
  ControlHandle saveButton = (ControlHandle)itemH;
  GetDialogItem(dlg, rCloseConfirm_CancelButtonIndex, &type, &itemH, &box);
  ControlHandle cancelButton = (ControlHandle)itemH;
  GetDialogItem(dlg, rCloseConfirm_DontSaveButtonIndex, &type, &itemH, &box);
  ControlHandle dontSaveButton = (ControlHandle)itemH;
  if (spec) {
    ParamText(spec->name, "\p", "\p", "\p");
  } else {
    ParamText("\puntitled", "\p", "\p", "\p");
  }

  SetCursor(&qd.arrow);
  SetDialogDefaultItem(dlg, rCloseConfirm_SaveButtonIndex);
  ShowWindow(dlg);

  short item;
  // If we had any interactions that weren't dialog-closing buttons,
  // we'd have a loop around ModalDialog.
  ModalDialog(NULL, &item);
  FlushEvents(everyEvent, -1);
  DisposeDialog(dlg);
  return item;
}

Boolean closeConfirmForDoc(DocumentPeek doc) {
  if (!doc->dirty) {
    return true;
  }

  switch (closeConfirm(doc->fsSpecSet ? &doc->fsSpec : NULL)) {
    case rCloseConfirm_SaveButtonIndex: {
      // try to save. if we have an associated file, simply save over.
      if (doc->fsSpecSet) {
        DoSave(doc);
        return true;
      } else {
        // otherwise, do a save-as. If save goes through, can close.
        return DoSaveAs(doc);
      }
    } break;
    case rCloseConfirm_CancelButtonIndex:
      return false;
    case rCloseConfirm_DontSaveButtonIndex:
      return true;
  }
}

Boolean closeConfirmForWin(WindowPtr window) {
  TwelfWinPtr twin = (TwelfWinPtr)window;
  switch (twin->winType) {
    case TwelfWinDocument: {
      return closeConfirmForDoc(getDoc(window));
    } break;
    case TwelfWinAbout: {
      return true;
    } break;
  }
}
