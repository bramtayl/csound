#include "csound_type_system_internal.h"

#include "aops.h"              // for ASSIGN
#include "csound.h"            // for csoundGetTypeForArg, CS_TYPE, csoundW...
#include "csoundCore_common.h" // for NOTOK, OK
#include "text.h"

/* GENERIC VARIABLE COPYING */
int copyVarGeneric(CSOUND *csound, void *p) {
  ASSIGN *assign = (ASSIGN *)p;
  CS_TYPE *typeR = csoundGetTypeForArg(assign->r);
  CS_TYPE *typeA = csoundGetTypeForArg(assign->a);

  if (typeR != typeA) {
    csoundWarning(csound,
                  Str("error: = opcode given variables with two different "
                      "types: %s : %s\n"),
                  typeR->varTypeName, typeA->varTypeName);
    return NOTOK;
  }

  typeR->copyValue(csound, typeR, assign->r, assign->a);
  return OK;
}