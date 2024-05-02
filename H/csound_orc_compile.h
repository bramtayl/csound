#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

int csoundCompileTreeInternal(CSOUND *csound, TREE *root, int async);

/**
   Parse and compile an orchestra given on an string (OPTIONAL)
   if str is NULL the string is taken from the internal corfile
   containing the initial orchestra file passed to Csound.
   Also evaluates any global space code.
   async determines asynchronous operation of the
   merge stage.
*/
int csoundCompileOrcInternal(CSOUND *csound, const char *str, int async);

uint8_t file_to_int(CSOUND*, const char*);

int argsRequired(char* argString);

char** splitArgs(CSOUND* csound, char* argString);

void free_instrtxt(CSOUND *csound, INSTRTXT *instrtxt);

char argtyp2(char *s);

int tree_arg_list_count(TREE *root);

int pnum(char *s);


#ifdef __cplusplus
}
#endif /* __cplusplus */