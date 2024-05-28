/*
    dumpf.h:

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

                                                        /*  DUMPF.H  */
#pragma once

#include "csoundCore_common.h"

typedef struct {
        OPDS   h;
        MYFLT  *ksig, *ifilcod, *iformat, *iprd;
        int    format;
        int32   countdown, timcount;
        FILE   *f;
        FDCH   fdch;
} KDUMP;

typedef struct {
        OPDS   h;
        MYFLT  *ksig1, *ksig2, *ifilcod, *iformat, *iprd;
        int    format;
        int32   countdown, timcount;
        FILE   *f;
        FDCH   fdch;
} KDUMP2;

typedef struct {
        OPDS   h;
        MYFLT  *ksig1, *ksig2, *ksig3, *ifilcod, *iformat, *iprd;
        int    format;
        int32   countdown, timcount;
        FILE   *f;
        FDCH   fdch;
} KDUMP3;

typedef struct {
        OPDS   h;
        MYFLT  *ksig1, *ksig2, *ksig3, *ksig4, *ifilcod, *iformat, *iprd;
        int    format;
        int32   countdown, timcount;
        FILE   *f;
        FDCH   fdch;
} KDUMP4;

typedef struct {
        OPDS   h;
        MYFLT  *k1, *ifilcod, *iformat, *iprd;
        /* MYFLT  *interp; */
        int    format;
        int32   countdown, timcount;
        MYFLT  k[4];
        FILE   *f;
        FDCH   fdch;
} KREAD;

typedef struct {
        OPDS   h;
        MYFLT  *k1, *k2, *ifilcod, *iformat, *iprd;
        /* MYFLT  *interp; */
        int    format;
        int32   countdown, timcount;
        MYFLT  k[4];
        FILE   *f;
        FDCH   fdch;
} KREAD2;

typedef struct {
        OPDS   h;
        MYFLT  *k1, *k2, *k3, *ifilcod, *iformat, *iprd;
        /* MYFLT  *interp; */
        int    format;
        int32   countdown, timcount;
        MYFLT  k[4];
        FILE   *f;
        FDCH   fdch;
} KREAD3;

typedef struct {
        OPDS   h;
        MYFLT  *k1, *k2, *k3, *k4, *ifilcod, *iformat, *iprd;
        /* MYFLT  *interp; */
        int    format;
        int32  countdown, timcount;
        MYFLT  k[4];
        FILE   *f;
        FDCH   fdch;
} KREAD4;

typedef struct {
        OPDS   h;
        STRINGDAT  *str;
        MYFLT *ifilcod, *iprd;
        int32  countdown, timcount;
        char   *lasts;
        FILE   *f;
        FDCH   fdch;
} KREADS;


int32_t kdmpset_p(CSOUND *, KDUMP *), kdmp2set_p(CSOUND *, KDUMP2 *);
int32_t kdmp3set_p(CSOUND *, KDUMP3 *), kdmp4set_p(CSOUND *, KDUMP4 *);
int32_t kdmpset_S(CSOUND *, KDUMP *), kdmp2set_S(CSOUND *, KDUMP2 *);
int32_t kdmp3set_S(CSOUND *, KDUMP3 *), kdmp4set_S(CSOUND *, KDUMP4 *);
int32_t kdump(CSOUND *, KDUMP *), kdump2(CSOUND *, KDUMP2 *);
int32_t kdump3(CSOUND *, KDUMP3 *), kdump4(CSOUND *, KDUMP4 *);
int32_t krdset_S(CSOUND *, KREAD *), krd2set_S(CSOUND *, KREAD2 *);
int32_t krd3set_S(CSOUND *, KREAD3 *), krd4set_S(CSOUND *, KREAD4 *);
int32_t krdset_p(CSOUND *, KREAD *), krd2set_p(CSOUND *, KREAD2 *);
int32_t krd3set_p(CSOUND *, KREAD3 *), krd4set_p(CSOUND *, KREAD4 *);
int32_t kread(CSOUND *, KREAD *), kread2(CSOUND *, KREAD2 *);
int32_t kread3(CSOUND *, KREAD3 *), kread4(CSOUND *, KREAD4 *);
int32_t krdsset_S(CSOUND *, KREADS *),krdsset_p(CSOUND *, KREADS *),
        kreads(CSOUND *, KREADS *);