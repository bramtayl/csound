/*
    pvoc.h:

    Copyright (c) 2005 Istvan Varga

    This file is part of Csound.

    The Csound Library is free software; you can redistribute it
    and/or modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    Csound is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Csound; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
    02110-1301 USA
*/

#ifndef CSOUND_PVOC_H
#define CSOUND_PVOC_H

// #include "csdl.h"
#include "csoundCore_internal.h"
#include "interlocks.h"

typedef struct PVOC_GLOBALS_ PVOC_GLOBALS;

#include "dsputil.h"
#include "pvread.h"
#include "pvadd.h"
#include "pvocext.h"
#include "namedins_public.h"

typedef struct {
    OPDS    h;
    MYFLT   *ktimpnt, *ifilno;
    int32   maxFr, frSiz, prFlg;
    /* base Frame (in frameData0) and maximum frame on file, ptr to fr, size */
    MYFLT   frPktim, frPrtim, asr, scale;
    float   *frPtr;
    AUXCH   auxch;
    MYFLT   *lastPhase, *fftBuf;  /* [PVFFTSIZE] FFT works on Real & Imag */
    MYFLT   *buf;
} PVBUFREAD;

typedef struct {
    FUNC    *function, *nxtfunction;
    MYFLT   d;
    int32   cnt;
} TSEG;

typedef struct {
    OPDS    h;
    MYFLT   *argums[VARGMAX];
    TSEG    *cursegp;
    FUNC    *outfunc;
    int32   nsegs;
    AUXCH   auxch;
} TABLESEG;

struct PVOC_GLOBALS_ {
    CSOUND    *csound;
    MYFLT     *dsputil_sncTab;
    PVBUFREAD *pvbufreadaddr;
    TABLESEG  *tbladr;
};

extern PVOC_GLOBALS *PVOC_AllocGlobals(CSOUND *csound);

static inline PVOC_GLOBALS *PVOC_GetGlobals(CSOUND *csound)
{
    PVOC_GLOBALS  *p;

    p = (PVOC_GLOBALS*) csoundQueryGlobalVariable(csound, "pvocGlobals");
    if (p == NULL)
      return PVOC_AllocGlobals(csound);
    return p;
}

typedef struct {
    OPDS    h;
    MYFLT   *rslt, *ktimpnt, *kfmod, *ifilno, *ispecwp, *imode;
    MYFLT   *ifreqlim, *igatefun;
    int32   mems;
    int32   kcnt, baseFr, maxFr, frSiz, prFlg, opBpos;
    /* RWD 8:2001 for pvocex: need these too */
    int32   frInc, chans;

    MYFLT   frPktim, frPrtim, scale, asr, lastPex;
    MYFLT   PvMaxAmp;
    float   *frPtr, *pvcopy;
    FUNC    *AmpGateFunc;
    AUXCH   auxch;
    MYFLT   *lastPhase; /* [PVDATASIZE] Keep track of cum. phase */
    MYFLT   *fftBuf;    /* [PVFFTSIZE]  FFT works on Real & Imag */
    MYFLT   *dsBuf;     /* [PVFFTSIZE]  Output of downsampling may be 2x */
    MYFLT   *outBuf;    /* [PVFFTSIZE]  Output buffer over win length */
    MYFLT   *window;    /* [PVWINLEN]   Store 1/2 window */
    MYFLT   *dsputil_env;
    AUXCH   memenv;
    PVOC_GLOBALS  *pp;
} PVOC;

#endif  /* CSOUND_PVOC_H */
