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
#include "namedins_public.h"

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
      csound->ErrorMsg(csound, Str("instr %s not found"), s);
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
          csound->InitError(csound, Str("Cannot Find Instrument %d"), (int) insno);
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
      csound->InitError(csound,
                        Str("cannot find the specified instrument or opcode"));
      insno = NOT_AN_INSTRUMENT;
    }
    return insno;
}

/* ----------------------------------------------------------------------- */
/* the following functions are for efficient management of the opcode list */





/* -------- IV - Jan 29 2005 -------- */


/**
 * Allocate nbytes bytes of memory that can be accessed later by calling
 * csoundQueryGlobalVariable() with the specified name; the space is
 * cleared to zero.
 * Returns CSOUND_SUCCESS on success, CSOUND_ERROR in case of invalid
 * parameters (zero nbytes, invalid or already used name), or
 * CSOUND_MEMORY if there is not enough memory.
 */
PUBLIC int csoundCreateGlobalVariable(CSOUND *csound,
                                      const char *name, size_t nbytes)
{
    void* p;
    /* create new empty database if it does not exist yet */
    if (UNLIKELY(csound->namedGlobals == NULL)) {
      csound->namedGlobals = cs_hash_table_create(csound);
      if (UNLIKELY(csound->namedGlobals == NULL))
        return CSOUND_MEMORY;
    }
    /* check for valid parameters */
    if (UNLIKELY(name == NULL))
      return CSOUND_ERROR;
    if (UNLIKELY(name[0] == '\0'))
      return CSOUND_ERROR;
    if (UNLIKELY(nbytes < (size_t) 1 || nbytes >= (size_t) 0x7F000000L))
      return CSOUND_ERROR;

    if (cs_hash_table_get(csound, csound->namedGlobals, (char*)name) != NULL)
      return CSOUND_ERROR;

    p = csound->Calloc(csound, nbytes);
    if (UNLIKELY(p == NULL))
      return CSOUND_MEMORY;

    cs_hash_table_put(csound, csound->namedGlobals, (char*)name, p);
    return CSOUND_SUCCESS;
}

/**
 * Get pointer to space allocated with the name "name".
 * Returns NULL if the specified name is not defined.
 */
PUBLIC void *csoundQueryGlobalVariable(CSOUND *csound, const char *name)
{
    /* check if there is an actual database to search */
    if (csound->namedGlobals == NULL) return NULL;

    /* check for a valid name */
    if (UNLIKELY(name == NULL)) return NULL;
    if (UNLIKELY(name[0] == '\0')) return NULL;

    return cs_hash_table_get(csound, csound->namedGlobals, (char*) name);
}

/**
 * This function is the same as csoundQueryGlobalVariable(), except the
 * variable is assumed to exist and no error checking is done.
 * Faster, but may crash or return an invalid pointer if 'name' is
 * not defined.
 */
PUBLIC void *csoundQueryGlobalVariableNoCheck(CSOUND *csound, const char *name)
{
    return cs_hash_table_get(csound, csound->namedGlobals, (char*) name);
}

/**
 * Free memory allocated for "name" and remove "name" from the database.
 * Return value is CSOUND_SUCCESS on success, or CSOUND_ERROR if the name is
 * not defined.
 */
PUBLIC int csoundDestroyGlobalVariable(CSOUND *csound, const char *name)
{
    void *p = cs_hash_table_get(csound, csound->namedGlobals, (char*)name);
    if (UNLIKELY(p == NULL))
      return CSOUND_ERROR;

    csound->Free(csound, p);
    cs_hash_table_remove(csound, csound->namedGlobals, (char*) name);

    return CSOUND_SUCCESS;
}

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
