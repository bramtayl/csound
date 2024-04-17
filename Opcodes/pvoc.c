/*
    pvoc.c:

    Copyright (c) 2006 Istvan Varga

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

#include "pvoc.h"

int32_t     pvset(CSOUND *, void *), pvset_S(CSOUND *, void *);
int32_t     pvoc(CSOUND *, void *);
int32_t     pvaddset(CSOUND *, void *), pvadd(CSOUND *, void *);
int32_t     pvaddset_S(CSOUND *, void *);
int32_t     tblesegset(CSOUND *, void *), ktableseg(CSOUND *, void *);
int32_t     ktablexseg(CSOUND *, void *);
int32_t     vpvset(CSOUND *, void *), vpvset_S(CSOUND *, void *),
        vpvoc(CSOUND *, void *);
int32_t     pvreadset(CSOUND *, void *), pvread(CSOUND *, void *);
int32_t     pvcrossset(CSOUND *, void *), pvcross(CSOUND *, void *);
int32_t     pvbufreadset(CSOUND *, void *), pvbufread(CSOUND *, void *);
int32_t     pvinterpset(CSOUND *, void *), pvinterp(CSOUND *, void *);

int32_t     pvreadset_S(CSOUND *, void *);
int32_t     pvcrossset_S(CSOUND *, void *);
int32_t     pvbufreadset_S(CSOUND *, void *);
int32_t     pvinterpset_S(CSOUND *, void *);

#define S(x)    sizeof(x)

static OENTRY pvoc_localops[] =
  {
   { "pvoc",      S(PVOC),      0, 3, "a",  "kkSoooo", pvset_S, pvoc, NULL, NULL},
   { "pvoc.i",      S(PVOC),      0, 3, "a",  "kkioooo", pvset, pvoc, NULL, NULL},
{ "tableseg",  S(TABLESEG),  TR, 3, "",   "iim",     tblesegset, ktableseg, NULL, NULL},
{ "ktableseg", S(TABLESEG),  _QQ|TR, 3, "",   "iim",  tblesegset, ktableseg, NULL, NULL},
{ "tablexseg", S(TABLESEG),  TW, 3, "",   "iin",     tblesegset, ktablexseg, NULL, NULL},
   { "vpvoc",     S(VPVOC),     TR, 3, "a",  "kkSoo",   vpvset_S, vpvoc, NULL, NULL},
   { "vpvoc.i",     S(VPVOC),     TR, 3, "a",  "kkioo",   vpvset, vpvoc, NULL, NULL},
{ "pvread",    S(PVREAD),  0,  3, "kk", "kSi",     pvreadset_S, pvread, NULL, NULL},
{ "pvread.i",    S(PVREAD),  0,  3, "kk", "kii",     pvreadset, pvread, NULL, NULL},
   { "pvcross",   S(PVCROSS), 0,  3, "a",  "kkSkko",  pvcrossset_S, pvcross, NULL, NULL},
{ "pvbufread", S(PVBUFREAD),0, 3, "",   "kS",      pvbufreadset_S, pvbufread, NULL, NULL},
   { "pvinterp",  S(PVINTERP), 0, 3, "a",  "kkSkkkkkk", pvinterpset_S, pvinterp, NULL, NULL},
   { "pvcross.i",   S(PVCROSS), 0,  3, "a",  "kkikko",  pvcrossset, pvcross, NULL, NULL},
{ "pvbufread.i", S(PVBUFREAD),0, 3, "",   "ki",      pvbufreadset, pvbufread, NULL, NULL},
   { "pvinterp.i",  S(PVINTERP), 0, 3, "a",  "kkikkkkkk", pvinterpset, pvinterp, NULL, NULL},
   { "pvadd",     S(PVADD),   0,  3, "a",  "kkSiiopooo", pvaddset_S, pvadd, NULL, NULL},
   { "pvadd.i",     S(PVADD),   0,  3, "a",  "kkiiiopooo", pvaddset, pvadd, NULL, NULL}
};

PVOC_GLOBALS *PVOC_AllocGlobals(CSOUND *csound)
{
    PVOC_GLOBALS  *p;
#ifdef BETA
    csoundMessage(csound, "calling alloc globals");
#endif
    if (UNLIKELY(csoundCreateGlobalVariable(csound, "pvocGlobals",
                                              sizeof(PVOC_GLOBALS)) != 0)){
      csoundErrorMsg(csound, Str("Error allocating PVOC globals"));
      return NULL;
    }
    p = (PVOC_GLOBALS*) csoundQueryGlobalVariable(csound, "pvocGlobals");
    p->csound = csound;
    p->dsputil_sncTab = (MYFLT*) NULL;
    p->pvbufreadaddr = (PVBUFREAD*) NULL;
    p->tbladr = (TABLESEG*) NULL;

    return p;
}

LINKAGE_BUILTIN(pvoc_localops)

