#include "find_opcode.h"
#include "csound.h"
#include "csound_orc.h"
#include "csound_orc_semantics.h"
#include "memalloc.h"
#include "prototyp.h"
#include <ctype.h>

char *get_opcode_short_name(CSOUND *csound, char *opname) {

  char *dot = strchr(opname, '.');
  if (dot != NULL) {
    int opLen = dot - opname;
    return cs_strndup(csound, opname, opLen);
  }
  return opname;
}

OENTRY *find_opcode_new(CSOUND *csound, char *opname, char *outArgsFound,
                        char *inArgsFound) {

  //    csoundMessage(csound, "Searching for opcode: %s | %s | %s\n",
  //                    outArgsFound, opname, inArgsFound);

  OENTRIES *opcodes = find_opcode2(csound, opname);

  if (opcodes->count == 0) {
    return NULL;
  }
  OENTRY *retVal = resolve_opcode(csound, opcodes, outArgsFound, inArgsFound);

  mfree(csound, opcodes);
  return retVal;
}

static OENTRY *resolve_opcode_exact(CSOUND *csound, OENTRIES *entries,
                                    char *outArgTypes, char *inArgTypes) {
  IGN(csound);
  OENTRY *retVal = NULL;
  int i;

  char *outTest = (!strcmp("0", outArgTypes)) ? "" : outArgTypes;
  for (i = 0; i < entries->count; i++) {
    OENTRY *temp = entries->entries[i];
    if (temp->intypes != NULL && !strcmp(inArgTypes, temp->intypes) &&
        temp->outypes != NULL && !strcmp(outTest, temp->outypes)) {
      retVal = temp;
    }
  }
  return retVal;
}

OENTRY *find_opcode_exact(CSOUND *csound, char *opname, char *outArgsFound,
                          char *inArgsFound) {

  OENTRIES *opcodes = find_opcode2(csound, opname);

  if (opcodes->count == 0) {
    return NULL;
  }

  OENTRY *retVal =
      resolve_opcode_exact(csound, opcodes, outArgsFound, inArgsFound);

  mfree(csound, opcodes);

  return retVal;
}

/* find opcode with the specified name in opcode list */
/* returns index to opcodlst[], or zero if the opcode cannot be found */
OENTRY *find_opcode(CSOUND *csound, char *opname) {
  char *shortName;
  CONS_CELL *head;
  OENTRY *retVal;

  if (opname[0] == '\0' || isdigit(opname[0]))
    return 0;

  shortName = get_opcode_short_name(csound, opname);

  head = cs_hash_table_get(csound, csound->opcodes, shortName);

  retVal = (head != NULL) ? head->value : NULL;
  if (shortName != opname)
    mfree(csound, shortName);

  return retVal;
}