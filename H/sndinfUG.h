/*
    sndinfUG.h:

    Copyright (C) 1999 matt ingalls

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

#pragma once

#include "csoundCore_common.h"

typedef struct {
    OPDS    h;
    MYFLT   *r1, *ifilno, *irawfiles;
} SNDINFO;

typedef struct {
    OPDS    h;
    MYFLT   *r1, *ifilno, *channel;
} SNDINFOPEAK;

typedef struct {
    OPDS  h;
    MYFLT *r1, *ifilno;
} FILEVALID;

int32_t filelen(CSOUND *, SNDINFO *), filenchnls(CSOUND *, SNDINFO *);
int32_t filesr(CSOUND *, SNDINFO *), filepeak(CSOUND *, SNDINFOPEAK *);
int32_t filevalid(CSOUND *, FILEVALID *);
int32_t filelen_S(CSOUND *, SNDINFO *), filenchnls_S(CSOUND *, SNDINFO *);
int32_t filesr_S(CSOUND *, SNDINFO *), filepeak_S(CSOUND *, SNDINFOPEAK *);
int32_t filevalid_S(CSOUND *, FILEVALID *);
int32_t filebit(CSOUND *, SNDINFO *); int32_t filebit_S(CSOUND *, SNDINFO *);