/*
    namedins.c:

    Copyright (C) 2002, 2005, 2006 Istvan Varga

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

#include "csoundCore_internal.h"
#include "namedins.h"
#include "csound_orc_semantics.h"
#include <ctype.h>
#include "rdscor.h"
#include "namedins_public.h"
#include "memalloc.h"
#include "fgens_public.h"
#include "insert_public.h"
#include "text.h"

/* check if the string s is a valid instrument or opcode name */
/* return value is zero if the string is not a valid name */

int check_instr_name(char *s)
{
    char    *c = s;

    if (UNLIKELY(!*c)) return 0;  /* empty */
    if (UNLIKELY(!isalpha(*c) &&
                 *c != '_')) return 0;    /* chk if 1st char is valid */
    while (*++c)
      if (UNLIKELY(!isalnum(*c) && *c != '_')) return 0;
    return 1;   /* no errors */
}


MYFLT  named_instr_find_in_engine(CSOUND *csound, char *s,
                                  ENGINE_STATE *engineState) {

  INSTRNAME     *inm;
    int ss = (*s=='-'?1:0);
    char *tt;
    if (!engineState->instrumentNames)
      return 0L;                              /* no named instruments defined */
    if ((tt=strchr(s, '.'))==NULL) {
      /* now find instrument */
      inm = cs_hash_table_get(csound, engineState->instrumentNames, s+ss);
      return (inm == NULL) ? 0L : (ss ? -inm->instno : inm->instno);
    }
    else {                     /* tagged instrument */
      char buff[256];
      int len = tt-s-ss;
      MYFLT frac;
      strncpy(buff,s+ss, len);
      buff[len] = '\0';
      inm = cs_hash_table_get(csound, engineState->instrumentNames, buff);
      frac = atof(tt);
      // printf("** fraction found %f\n", frac);
      return (inm == NULL) ? 0L : (ss ? -(inm->instno+frac) : inm->instno+frac);
    }
}
/* find the instrument number for the specified name */
/* return value is zero if none was found */

MYFLT named_instr_find(CSOUND *csound, char *s)
{
  return named_instr_find_in_engine(csound, s, &csound->engineState);
}

/* same as strarg2insno, but runs at perf time, */
/* and does not support numbered instruments */
/* (used by opcodes like event or schedkwhen) */
int32 strarg2insno_p(CSOUND *csound, char *s)
{
    int32    insno;

    if (UNLIKELY(!(insno = named_instr_find(csound, s)))) {
      csoundErrorMsg(csound, Str("instr %s not found"), s);
      return NOT_AN_INSTRUMENT;
    }
    return insno;
}

/* convert opcode string argument to instrument number */
/* (also allows user defined opcode names); if the integer */
/* argument is non-zero, only opcode names are searched */
/* return value is -1 if the instrument cannot be found */
/* (in such cases, csoundInitError() is also called) */
int32 strarg2opcno(CSOUND *csound, void *p, int is_string, int force_opcode)
{
    int32    insno = 0;

    if (!force_opcode) {        /* try instruments first, if enabled */
      if (is_string) {
        insno = named_instr_find(csound, (char*) p);
      }
      else {      /* numbered instrument */
        insno = (int32) *((MYFLT*) p);
        if (UNLIKELY(insno < 1 || insno > csound->engineState.maxinsno ||
                     !csound->engineState.instrtxtp[insno])) {
          csoundInitError(csound, Str("Cannot Find Instrument %d"), (int) insno);
          return NOT_AN_INSTRUMENT;
        }
      }
    }
    if (!insno && is_string) {              /* if no instrument was found, */
      OPCODINFO *inm = csound->opcodeInfo;  /* search for user opcode */
      while (inm && sCmp(inm->name, (char*) p)) inm = inm->prv;
      if (inm) insno = (int32) inm->instno;
    }
    if (UNLIKELY(insno < 1)) {
      csoundInitError(csound,
                        Str("cannot find the specified instrument or opcode"));
      insno = NOT_AN_INSTRUMENT;
    }
    return insno;
}

/* ----------------------------------------------------------------------- */
/* the following functions are for efficient management of the opcode list */





/* -------- IV - Jan 29 2005 -------- */

/**
 * Free entire global variable database. This function is for internal use
 * only (e.g. by RESET routines).
 */
void csoundDeleteAllGlobalVariables(CSOUND *csound)
{
    if (csound == NULL || csound->namedGlobals == NULL) return;

    cs_hash_table_mfree_complete(csound, csound->namedGlobals);
    csound->namedGlobals = NULL;
}
