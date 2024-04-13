#include "document.h"

Boolean isReadOnly(DocType docType) {
  switch (docType) {
    case TwelfDocument:
      return false;
    case TwelfOutput:
      return true;
    case TwelfLog:
      return true;
    default:
      return true;
  }
}
