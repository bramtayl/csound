/*
    rdscor.c:

    Copyright (C) 2011 John ffitch (after Barry Vercoe)

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

#include "rdscor.h"

#include "csoundCore_internal.h"         /*                  RDSCORSTR.C */
#include "corfile.h"
#include "insert.h"
#include "rdscor.h"
#include "memalloc.h"
#include "text.h"
#include "rdscor_internal.h"
#include "fgens.h"

char* get_arg_string(CSOUND *csound, MYFLT p)
{
    int32 n;
    char *ss;
    INSDS* ip = csound->ids->insdshead;
    while (ip->opcod_iobufs != NULL) {
      ip = ((OPCOD_IOBUFS*)ip->opcod_iobufs)->parent_ip;
    }
    ss = ip->strarg;  /* look at this instr's strarg */

#ifdef USE_DOUBLE
    {
      union {
        MYFLT d;
        int32 i[2];
      } ch;
      ch.d = p;
      if (byte_order()==0)
        n = ch.i[1]&0xffff;
      else
        n = ch.i[0]&0xffff;
      //printf("UNION %.8x %.8x\n", ch.i[0], ch.i[1]);
    }
#else
    {
      union {
        MYFLT d;
        int32 j;
      } ch;
      ch.d = p; n = ch.j&0xffff;
      //printf("SUNION %.8x \n", ch.j);
    }
#endif
    while (n-- > 0) {
      ss += strlen(ss)+1;
    }
    //printf("*** -> %s\n", ss);
    return ss;
}
