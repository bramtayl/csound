/*
    fgens.h:

    Copyright (C) 1991 Barry Vercoe

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
                                                /*      FGENS.H         */

#ifndef CSOUND_FGENS_H
#define CSOUND_FGENS_H

#include "csound.h"
#include "resize.h"

#define MAXFNUM 100
#define GENMAX  60
#define FTAB_SEARCH_BASE (100)

typedef struct namedgen {
    char    *name;
    int     genum;
    struct namedgen *next;
} NAMEDGEN;

inline int32_t byte_order(void)
{
    const int32_t one = 1;
    return (!*((char*) &one));
};

inline unsigned int isPowerOfTwo (unsigned int x) {
  return (x > 0) && !(x & (x - 1)) ? 1 : 0;
};

int32_t resize_table(CSOUND *csound, RESIZE *p);
int allocgen(CSOUND *csound, char *s, GEN fn);
extern const GEN or_sub[];
CS_NOINLINE FUNC *ftalloc(const FGDATA *);
CS_NOINLINE void ftresdisp(const FGDATA *, FUNC *);
void generate_sine_tab(CSOUND *csound);
FUNC *csoundFTnp2Findint(CSOUND *csound, MYFLT *argp, int verbose);
FUNC *gen01_defer_load(CSOUND *csound, int fno);

#endif  /* CSOUND_FGENS_H */