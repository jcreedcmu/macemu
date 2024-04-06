#pragma once

#include <Files.h>

#include "document.h"

#define rCloseConfirm_SaveButtonIndex 1
#define rCloseConfirm_CancelButtonIndex 2
#define rCloseConfirm_DontSaveButtonIndex 3

short closeConfirm(FSSpec *spec);

short closeConfirmForDoc(DocumentPeek doc);
