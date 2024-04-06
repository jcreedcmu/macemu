#include "dialogs.h"

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

short closeConfirmForDoc(DocumentPeek doc) {
  if (doc->dirty) {
    return closeConfirm(doc->fsSpecSet ? &doc->fsSpec : NULL);
  } else {
    return rCloseConfirm_DontSaveButtonIndex;
  }
}
