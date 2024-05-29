#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Gets an alphabetically sorted list of all opcodes.
 * Should be called after externals are loaded by csoundCompile().
 * Returns the number of opcodes, or a negative error code on failure.
 * Make sure to call csoundDisposeOpcodeList() when done with the list.
 */
PUBLIC int csoundNewOpcodeList(CSOUND *, opcodeListEntry **opcodelist);

/**
 * Releases an opcode list.
 */
PUBLIC void csoundDisposeOpcodeList(CSOUND *, opcodeListEntry *opcodelist);

#ifdef __cplusplus
}
#endif