#include "namedins_public.h"

#include "fgens_public.h"
#include "memalloc.h"
#include "namedins.h"
#include "rdscor.h"

/* convert opcode string argument to instrument number */
/* return value is -1 if the instrument cannot be found */
/* (in such cases, csoundInitError() is also called) */
int32 strarg2insno(CSOUND *csound, void *p, int is_string) {
  int32 insno;

  if (is_string) {
    if (UNLIKELY((insno = named_instr_find(csound, (char *)p)) <= 0)) {
      csoundMessage(csound, Str("WARNING: instr %s not found\n"), (char *)p);
      return NOT_AN_INSTRUMENT;
    }
  } else { /* numbered instrument */
    insno = (int32) * ((MYFLT *)p);
    if (UNLIKELY(insno < 1 || insno > csound->engineState.maxinsno ||
                 !csound->engineState.instrtxtp[insno])) {
      csoundWarning(csound, Str("Cannot Find Instrument %d"), (int)insno);
      return csound->engineState.maxinsno;
    }
  }
  return insno;
}

/* create file name from opcode argument (string or MYFLT)      */
/*   CSOUND *csound:                                            */
/*      pointer to Csound instance                              */
/*   char *s:                                                   */
/*      output buffer, should have enough space; if NULL, the   */
/*      required amount of memory is allocated and returned     */
/*   void *p:                                                   */
/*      opcode argument, is interpreted as char* or MYFLT*,     */
/*      depending on the 'is_string' parameter                  */
/*   const char *baseName:                                      */
/*      name prefix to be used if the 'p' argument is MYFLT,    */
/*      and it is neither SSTRCOD, nor a valid index to strset  */
/*      space.                                                  */
/*      For example, if "soundin." is passed as baseName, file  */
/*      names in the format "soundin.%d" will be generated.     */
/*      baseName may be an empty string, but should not be NULL */
/*   int is_string:                                             */
/*      if non-zero, 'p' is interpreted as a char* pointer and  */
/*      is used as the file name. Otherwise, it is expected to  */
/*      point to a MYFLT value, and the following are tried:    */
/*        1. if the value is SSTRCOD, the string argument of    */
/*           the current score event is used (string p-field)   */
/*        2. if the value, rounded to the nearest integer, is a */
/*           valid index to strset space, the strset string is  */
/*           used                                               */
/*        3. the file name is generated using baseName and the  */
/*           value rounded to the nearest integer, as described */
/*           above                                              */
/*   return value:                                              */
/*      pointer to the output string; if 's' is not NULL, it is */
/*      always the same as 's', otherwise it is allocated with  */
/*      mmalloc() and the caller is responsible for      */
/*      freeing the allocated memory with mfree() or     */
/*      mfree()                                          */

char *strarg2name(CSOUND *csound, char *s, void *p, const char *baseName,
                  int is_string) {
  if (is_string) {
    /* opcode string argument */
    if (s == NULL) s = mmalloc(csound, strlen((char *)p) + 1);
    strcpy(s, (char *)p);
  } else if (isstrcod(*((MYFLT *)p))) {
    /* p-field string, unquote and copy */
    char *s2 = get_arg_string(csound, *((MYFLT *)p));
    int i = 0;
    // printf("strarg2name: %g %s\n", *((MYFLT*)p), s2);
    if (s == NULL) s = mmalloc(csound, strlen(s2) + 1);
    if (*s2 == '"') s2++;
    while (*s2 != '"' && *s2 != '\0') s[i++] = *(s2++);
    s[i] = '\0';
  } else {
    int i =
        (int)((double)*((MYFLT *)p) + (*((MYFLT *)p) >= FL(0.0) ? 0.5 : -0.5));
    if (i >= 0 && i <= (int)csound->strsmax && csound->strsets != NULL &&
        csound->strsets[i] != NULL) {
      if (s == NULL) s = mmalloc(csound, strlen(csound->strsets[i]) + 1);
      strcpy(s, csound->strsets[i]);
    } else {
      int n;
      if (s == NULL) {
        /* allocate +20 characters, assuming sizeof(int) <= 8 */
        s = mmalloc(csound, n = strlen(baseName) + 21);
        snprintf(s, n, "%s%d", baseName, i);
      } else
        sprintf(s, "%s%d", baseName, i); /* dubious */
    }
  }
  return s;
}