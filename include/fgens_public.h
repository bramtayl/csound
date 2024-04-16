#pragma once

#ifdef __BUILDING_LIBCSOUND
#include "csoundCore_internal.h"
#else
#include "csoundCore.h"
#endif

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

#ifdef __cplusplus
}
#endif /* __cplusplus */