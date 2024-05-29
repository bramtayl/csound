/*
    bus.h:

    Copyright (C) 2004 John ffitch
        (C) 2005, 2006 Istvan Varga
        (C) 2006 Victor Lazzarini

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

/*                                                      BUS.H           */

#pragma once

#ifndef CSOUND_BUS_H
#define CSOUND_BUS_H

#include "pstream.h"
#include "arrays.h"
#include "csound_standard_types.h"
#include "csoundCore_internal.h"

#define MAX_CHAN_NAME 1024

#ifdef USE_DOUBLE
#define MYFLT_INT_TYPE int64_t
#else
#define MYFLT_INT_TYPE int32_t
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    OPDS    h;
    MYFLT   *r, *a;
} CHNVAL;

typedef struct {
    OPDS    h;
    PVSDAT   *r;
    MYFLT    *a,*N, *overlap, *winsize, *wintype, *format;
    PVSDAT   init;
} FCHAN;

typedef struct {
    OPDS    h;
    MYFLT   *ans;
    MYFLT   *keyDown;
    int32_t evtbuf;
} KSENSE;

typedef struct channelEntry_s {
    struct channelEntry_s *nxt;
    controlChannelHints_t hints;
    MYFLT       *data;
    spin_lock_t lock;               /* Multi-thread protection */
    int32_t     type;
    int32_t     datasize;  /* size of allocated chn data */
    char        name[1];
} CHNENTRY;

typedef struct {
    OPDS        h;
    MYFLT       *arg;
    STRINGDAT   *iname;
    MYFLT       *fp;
    spin_lock_t *lock;
    int32_t     pos;
    char        chname[MAX_CHAN_NAME+1];
} CHNGET;

typedef struct {
    OPDS        h;
    ARRAYDAT    *arrayDat;
    STRINGDAT   *iname;
    MYFLT       *fp;
    spin_lock_t *lock;
    int32_t     pos;
    int32_t     arraySize;
    MYFLT**     channelPtrs;
    STRINGDAT   *channels;
    char        chname[MAX_CHAN_NAME+1];
} CHNGETARRAY;

typedef struct {
    OPDS    h;
    STRINGDAT   *iname[MAX_CHAN_NAME+1];
    MYFLT   *fp[MAX_CHAN_NAME+1];
    spin_lock_t *lock[MAX_CHAN_NAME+1];
} CHNCLEAR;

typedef struct {
    OPDS    h;
    STRINGDAT   *iname;
    MYFLT   *imode;
    MYFLT   *itype;
    MYFLT   *idflt;
    MYFLT   *imin;
    MYFLT   *imax;
    MYFLT   *ix;
    MYFLT   *iy;
    MYFLT   *iwidth;
    MYFLT   *iheight;
    STRINGDAT *Sattributes;
    spin_lock_t      *lock;
} CHN_OPCODE_K;

typedef struct {
    OPDS    h;
    STRINGDAT   *iname;
    MYFLT   *imode;
    spin_lock_t   *lock;
} CHN_OPCODE;

typedef struct {
    OPDS    h;
    MYFLT   *arg;
    STRINGDAT   *iname;
    MYFLT   *imode;
    MYFLT   *itype;
    MYFLT   *idflt;
    MYFLT   *imin;
    MYFLT   *imax;
} CHNEXPORT_OPCODE;

typedef struct {
    OPDS    h;
    MYFLT   *itype;
    MYFLT   *imode;
    MYFLT   *ictltype;
    MYFLT   *idflt;
    MYFLT   *imin;
    MYFLT   *imax;
    STRINGDAT   *iname;
} CHNPARAMS_OPCODE;

typedef struct {
    OPDS    h;
    MYFLT   *value, *valID;
    AUXCH   channelName;
    const CS_TYPE *channelType;
    void *channelptr;
} INVAL;

typedef struct {
    OPDS    h;
    MYFLT   *valID, *value;
    AUXCH   channelName;
    const CS_TYPE *channelType;
    void *channelptr;
} OUTVAL;

OPCODE_INIT_FUNCTION(bus_localops_init);
int32_t     chnget_array_opcode_perf_k(CSOUND *, CHNGETARRAY *);
int32_t     chnget_array_opcode_perf_a(CSOUND *, CHNGETARRAY *);
int32_t     chnget_array_opcode_perf_S(CSOUND* csound, CHNGETARRAY* p);
int32_t     chnset_array_opcode_perf_k(CSOUND *csound, CHNGETARRAY *p);
int32_t     chnset_array_opcode_perf_a(CSOUND *csound, CHNGETARRAY *p);
int32_t     chnset_array_opcode_perf_S(CSOUND *csound, CHNGETARRAY *p);
int32_t invalset(CSOUND *csound, INVAL *p);

inline CHNENTRY *find_channel(CSOUND *csound, const char *name)
{
    if (csound->chn_db != NULL && name[0]) {
        return (CHNENTRY*) cs_hash_table_get(csound, csound->chn_db, (char*) name);
    }
    return NULL;
}

CS_NOINLINE int32_t create_new_channel(CSOUND *csound, const char *name,
                                       int32_t type);

int32_t bus_cmp_func(const void *p1, const void *p2);

#ifdef __cplusplus
}
#endif

#endif      /* CSOUND_BUS_H */
