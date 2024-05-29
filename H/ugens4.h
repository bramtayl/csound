/*
    ugens4.h:

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

/*                                                      UGENS4.H        */

#pragma once

#include "csoundCore_internal.h"

typedef struct {
        OPDS    h;
        MYFLT   *ar, *xamp, *xcps, *knh, *ifn, *iphs;
        int16   ampcod, cpscod;
        int32    lphs;
        FUNC    *ftp;
        int     reported;
} BUZZ;

typedef struct {
        OPDS    h;
        MYFLT   *ar, *xamp, *xcps, *kn, *kk, *kr, *ifn, *iphs;
        int16   ampcod, cpscod, prvn;
        MYFLT   prvr, twor, rsqp1, rtn, rtnp1, rsumr;
        int32    lphs;
        FUNC    *ftp;
        int     reported;
        MYFLT   last;
} GBUZZ;

typedef struct {
        OPDS    h;
        MYFLT   *ar, *kamp, *kcps, *icps, *ifn, *imeth, *ipar1, *ipar2;
        MYFLT   sicps, param1, param2;
        int16   thresh1, thresh2, method;
        int32    phs256, npts, maxpts;
        AUXCH   auxch;
} PLUCK;

typedef struct {
        OPDS    h;
        MYFLT   *ar, *xamp, *iseed, *sel, *base;
        int     rand;
        int16   ampcod;
        int16   new;
} RAND;

typedef struct {
        OPDS    h;
        MYFLT   *ar, *xamp, *xcps, *iseed, *sel, *base;
        int16   ampcod, cpscod, new;
        int     rand;
        long    phs;
        MYFLT   num1;
} RANDH;

typedef struct {
        OPDS    h;
        MYFLT   *ar, *xamp, *xcps, *iseed, *sel, *base;
        int16   ampcod, cpscod, new;
        int     rand;
        long    phs;
        MYFLT   num1, num2, dfdmax;
} RANDI;

typedef struct {
        OPDS    h;
        MYFLT   *ar, *xamp, *xcps, *iseed, *sel, *base;
        int16   ampcod, cpscod, new;
        int     rand;
        int64_t phs;
        MYFLT   period, num1, num2;
        MYFLT   num3, num4;
} RANDC;

extern MYFLT intpow1(MYFLT x, int32_t n); /* Binary positive power function */
int32 randint31(int32 seed31);

int32_t bzzset(CSOUND *, BUZZ *), buzz(CSOUND *, BUZZ *);
int32_t gbzset(CSOUND *, GBUZZ *), gbuzz(CSOUND *, GBUZZ *);
int32_t plukset(CSOUND *, PLUCK *), pluck(CSOUND *, PLUCK *);
int32_t rndset(CSOUND *, RAND *), krand(CSOUND *, RAND *);
int32_t arand(CSOUND *, RAND *), rhset(CSOUND *, RANDH *);
int32_t krandh(CSOUND *, RANDH *), randh(CSOUND *, RANDH *);
int32_t riset(CSOUND *, RANDI *), krandi(CSOUND *, RANDI *);
int32_t rcset(CSOUND *, RANDC *), randc(CSOUND *, RANDC *);
int32_t krandc(CSOUND *, RANDC *);
int32_t randi(CSOUND *, RANDI *);
