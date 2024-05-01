#pragma once

#include "csoundCore.h"

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

/**
 * Allocates space for 'tableNum' with a length (not including the guard
 * point) of 'len' samples. The table data is not cleared to zero.
 * Return value is zero on success.
 */
PUBLIC int csoundFTAlloc(CSOUND *csound, int tableNum, int len);

/**
 * Deletes a function table.
 * Return value is zero on success.
 */
PUBLIC int csoundFTDelete(CSOUND *csound, int tableNum);

PUBLIC int isstrcod(MYFLT);

/**
 * Create ftable using evtblk data, and store pointer to new table in *ftpp.
 * If mode is zero, a zero table number is ignored, otherwise a new table
 * number is automatically assigned.
 * Returns zero on success.
 */
PUBLIC int hfgens(CSOUND *csound, FUNC **ftpp, const EVTBLK *evtblkp, int mode);

PUBLIC FUNC *csoundFTFind(CSOUND *, MYFLT *);
PUBLIC FUNC *csoundFTFindP(CSOUND *, MYFLT *);
PUBLIC FUNC *csoundFTnp2Find(CSOUND *, MYFLT *);

PUBLIC int fterror(const FGDATA *ff, const char *s, ...);

PUBLIC FUNC *csoundFTnp2Finde(CSOUND *, MYFLT *);

/**
 * Stores pointer to function table 'tableNum' in *tablePtr,
 * and returns the table length (not including the guard point).
 * If the table does not exist, *tablePtr is set to NULL and
 * -1 is returned.
 */
PUBLIC int csoundGetTable(CSOUND *, MYFLT **tablePtr, int tableNum);

/**
 * Stores pointer to the arguments used to generate
 * function table 'tableNum' in *argsPtr,
 * and returns the number of arguments used.
 * If the table does not exist, *argsPtr is set to NULL and
 * -1 is returned.
 * NB: the argument list starts with the GEN number and is followed by
 * its parameters. eg. f 1 0 1024 10 1 0.5  yields the list {10.0,1.0,0.5}
 */
PUBLIC int csoundGetTableArgs(CSOUND *csound, MYFLT **argsPtr, int tableNum);

/**
 * Checks if a given GEN number num is a named GEN
 * if so, it returns the string length (excluding terminating NULL char)
 * Otherwise it returns 0.
 */
PUBLIC int csoundIsNamedGEN(CSOUND *csound, int num);

/**
 * Gets the GEN name from a number num, if this is a named GEN
 * The final parameter is the max len of the string (excluding termination)
 */
PUBLIC void csoundGetNamedGEN(CSOUND *csound, int num, char *name, int len);

#ifdef __cplusplus
}
#endif /* __cplusplus */