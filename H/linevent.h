/*
    linevent.h:

    Copyright (C) 2001 matt ingalls

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

#ifndef CSOUND_LINEVENT_H
#define CSOUND_LINEVENT_H

#include "csoundCore_internal.h"

/*****************************************************************/
/* linevent                                                      */
/* Dec 2001 by matt ingalls                                      */
/*****************************************************************/
typedef struct {
    OPDS   h;
    MYFLT  *args[VARGMAX];
    int argno;
    int flag;
} LINEVENT;


typedef struct {
    OPDS   h;
    MYFLT  *inst;
    MYFLT  *args[VARGMAX];
    int argno;
} LINEVENT2;

#define STA_LINEVENT(x)   (csound->lineventStatics.x)

#define LBUFSIZ1 32768

void sensLine(CSOUND *csound, void *userData);
void    RTLineset(CSOUND *);

int32_t eventOpcodeI_(CSOUND *csound, LINEVENT *p, int32_t s, char p1);
int32_t eventOpcode_(CSOUND *csound, LINEVENT *p, int32_t s, char p1);

OPCODE_INIT_FUNCTION(linevent_localops_init);

#endif      /* CSOUND_LINEVENT_H */
