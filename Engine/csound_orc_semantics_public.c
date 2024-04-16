#include "csound_orc_semantics_public.h"

#include "memalloc.h"

char* cs_strdup(CSOUND* csound, char* str) {
  size_t len;
  char* retVal;

  if (str == NULL) return NULL;

  len = strlen(str);
  retVal = mmalloc(csound, len + 1);

  if (len > 0) {
    memcpy(retVal, str, len);
  }
  retVal[len] = '\0';

  return retVal;
}