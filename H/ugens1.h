/*
    ugens1.h:

    Copyright (C) 1991 Barry Vercoe, John ffitch

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

/*                                                         UGENS1.H        */

#pragma once

#include "csoundCore_common.h"

typedef struct {
        OPDS    h;
        MYFLT   *xr, *ia, *idur, *ib;
         double   val, incr, kincr;
} LINE;

typedef struct {
        OPDS    h;
        MYFLT   *xr, *ia, *idur, *ib;
        double   val, mlt, kmlt;
} EXPON;

typedef struct {
        int32  cnt, acnt;
         MYFLT  val, mlt, amlt;
} XSEG;

typedef struct {
        int32   cnt, acnt;
        double  nxtpt;
} SEG;

typedef struct {
        OPDS    h;
        MYFLT   *rslt, *argums[VARGMAX];
        SEG     *cursegp;
        int32   nsegs;
        int32   segsrem, curcnt;
        double  curval, curinc, curainc;
        AUXCH   auxch;
        int32   xtra;
} LINSEG;

typedef struct {
        OPDS    h;
        MYFLT   *rslt, *argums[VARGMAX];
        SEG     *cursegp;
        int32   nsegs;
        int32   segsrem, curcnt;
        double  y1, y2, x, inc, val;
        AUXCH   auxch;
        int32   xtra;
} COSSEG;

typedef struct {
        OPDS    h;
        MYFLT   *rslt, *argums[VARGMAX];
        SEG     *cursegp;
        int32   segsrem, curcnt;
        double  curval, curmlt, curamlt;
        int32   nsegs;
        AUXCH   auxch;
        int32   xtra;
} EXPSEG;

typedef struct {
        OPDS    h;
        MYFLT   *rslt, *argums[VARGMAX];
        XSEG    *cursegp;
        int32   segsrem, curcnt;
        double  curval, curmlt, curamlt;
        int32   nsegs;
        AUXCH   auxch;
} EXXPSEG;

typedef struct {
        OPDS    h;
        MYFLT   *rslt, *sig, *iris, *idur, *idec;
        double  lin1, inc1, lin2, inc2;
        int64_t  cnt1, cnt2;
} LINEN;

typedef struct {
        OPDS    h;
        MYFLT   *rslt, *sig, *iris, *idec, *iatdec;
        double  lin1, inc1, val, val2, mlt2;
        int64_t  cnt1;
} LINENR;

typedef struct {
        OPDS    h;
        MYFLT   *rslt, *xamp, *irise, *idur, *idec, *ifn, *iatss;
        MYFLT   *iatdec, *ixmod;
        int32   phs, ki, cnt1;
        double  val, mlt1, mlt2, asym;
        FUNC    *ftp;
} ENVLPX;

typedef struct {
        OPDS    h;
        MYFLT   *rslt, *xamp, *irise, *idec, *ifn, *iatss, *iatdec;
        MYFLT   *ixmod, *irind;
        int32   phs, ki, rlsing, rlscnt, rindep;
        double  val, mlt1, mlt2, asym, atdec;
        FUNC    *ftp;
} ENVLPR;

typedef struct {
        OPDS    h;
        MYFLT   *rslt, *argums[VARGMAX];
        XSEG    *cursegp;
        int32   nsegs;
        AUXCH   auxch;
} EXPSEG2;                         /*gab-A1*/

int32_t linset(CSOUND *, LINE *);
int32_t kline(CSOUND *, LINE *), aline(CSOUND *, LINE *);
int32_t expset(CSOUND *, EXPON *), kexpon(CSOUND *, EXPON *);
int32_t expon(CSOUND *, EXPON *), lsgset(CSOUND *, LINSEG *);
int32_t klnseg(CSOUND *, LINSEG *), linseg(CSOUND *, LINSEG *);
int32_t csgset(CSOUND *, COSSEG *), kosseg(CSOUND *, COSSEG *);
int32_t csgset_bkpt(CSOUND *, COSSEG *), cosseg(CSOUND *, COSSEG *);
int32_t csgrset(CSOUND *, COSSEG *);
int32_t kcssegr(CSOUND *, COSSEG *), cossegr(CSOUND *, COSSEG *);
int32_t madsrset(CSOUND *, LINSEG *), adsrset(CSOUND *, LINSEG *);
int32_t xdsrset(CSOUND *, EXXPSEG *), mxdsrset(CSOUND *, EXPSEG *);
int32_t expseg2(CSOUND *, EXPSEG2 *), xsgset(CSOUND *, EXXPSEG *);
int32_t kxpseg(CSOUND *, EXXPSEG *), expseg(CSOUND *, EXXPSEG *);
int32_t xsgset2(CSOUND *, EXPSEG2 *), lsgrset(CSOUND *, LINSEG *);
int32_t klnsegr(CSOUND *, LINSEG *), linsegr(CSOUND *, LINSEG *);
int32_t xsgrset(CSOUND *, EXPSEG *), kxpsegr(CSOUND *, EXPSEG *);
int32_t expsegr(CSOUND *, EXPSEG *), lnnset(CSOUND *, LINEN *);
int32_t klinen(CSOUND *, LINEN *), linen(CSOUND *, LINEN *);
int32_t lnrset(CSOUND *, LINENR *), klinenr(CSOUND *, LINENR *);
int32_t linenr(CSOUND *, LINENR *), evxset(CSOUND *, ENVLPX *);
int32_t knvlpx(CSOUND *, ENVLPX *), envlpx(CSOUND *, ENVLPX *);
int32_t evrset(CSOUND *, ENVLPR *), knvlpxr(CSOUND *, ENVLPR *);
int32_t envlpxr(CSOUND *, ENVLPR *);
int32_t lsgset_bkpt(CSOUND *csound, LINSEG *p);
int32_t xsgset_bkpt(CSOUND *csound, EXXPSEG *p);
int32_t xsgset_bkpt(CSOUND *csound, EXXPSEG *p), xsgset2b(CSOUND *, EXPSEG2 *);
int32_t alnnset(CSOUND *csound, LINEN *p);
int32_t alnrset(CSOUND *csound, LINENR *p);
int32_t aevxset(CSOUND *csound, ENVLPX *p);
int32_t aevrset(CSOUND *csound, ENVLPR *p);