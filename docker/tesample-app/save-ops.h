#pragma once

#include <Files.h>
#include <TextEdit.h>

#include "document.h"

OSErr writeFile(TEHandle te, FSSpec *spec);
OSErr readFile(TEHandle te, FSSpec *spec);
void associateFile(DocumentPeek doc, FSSpec *spec);
void DoSave(DocumentPeek doc);
Boolean DoSaveAs(DocumentPeek doc);  // true if saved, false if cancel
