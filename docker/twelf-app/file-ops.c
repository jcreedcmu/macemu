#include "file-ops.h"

#include "consts.h"
#include "windows.h"

// FIXME(fs): better error handling in all file operations
OSErr writeFile(TEHandle te, FSSpec *spec) {
  OSErr err;
  short refNum;
  err = FSpOpenDF(spec, fsRdWrPerm, &refNum);
  Handle textHandle = (Handle)TEGetText(te);
  long size = (*te)->teLength;
  err = SetFPos(refNum, fsFromStart, 0);
  err = FSWrite(refNum, &size, *textHandle);
  err = SetEOF(refNum, size);
  err = FSClose(refNum);
  FlushVol(nil, spec->vRefNum);
  return err;
}

OSErr readFile(TEHandle te, FSSpec *spec) {
  int16_t refNum;
  long textLength;
  OSErr err = FSpOpenDF(spec, fsCurPerm, &refNum);
  err = SetFPos(refNum, fsFromStart, 0);
  err = GetEOF(refNum, &textLength);
  if (textLength > kMaxTELength) {
    textLength = kMaxTELength;
  }
  Handle buf = NewHandle(textLength);
  err = FSRead(refNum, &textLength, *buf);
  for (int i = 0; i < textLength; i++) {
    if ((*buf)[i] == '\n') {
      (*buf)[i] = '\r';
    }
  }
  MoveHHi(buf);
  HLock(buf);
  TESetText(*buf, textLength, te);
  HUnlock(buf);
  DisposeHandle(buf);
  err = FSClose(refNum);
  return err;
}

void associateFile(DocumentPeek doc, FSSpec *spec) {
  doc->fsSpec = *spec;
  doc->fsSpecSet = true;
  SetWTitle((WindowPtr)doc, spec->name);
}

void DoSave(DocumentPeek doc) {
  writeFile(doc->docTE, &doc->fsSpec);
  doc->dirty = false;
}

Boolean DoSaveAs(DocumentPeek doc) {
  StandardFileReply reply;
  StandardPutFile("\pSave as:", "\puntitled", &reply);
  if (!reply.sfGood) {
    return false;
  }

  FSSpec *spec = &reply.sfFile;
  FSpCreate(spec, 'TWLF', 'TEXT', smSystemScript);
  writeFile(doc->docTE, spec);
  associateFile(doc, spec);
  doc->dirty = false;
  return true;
}

void openFileSpec(FSSpec *spec) {
  WindowPtr window = mkDocumentWindow(TwelfDocument);
  if (window == NULL) return;
  DocumentPeek doc = (DocumentPeek)window;
  TEHandle te = doc->docTE;

  readFile(te, spec);

  associateFile(getDoc(window), spec);

  ShowWindow(window);
  InvalRect(&window->portRect);
}
