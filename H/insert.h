/*
    insert.h:

    Copyright (C) 1991, 2002 Barry Vercoe, Istvan Varga

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

#include "csoundCore_internal.h"
#include "aops.h"

typedef struct {                        /*       INSERT.H                */
    OPDS    h;
    LBLBLK  *lblblk;
} GOTO;

typedef struct {
    OPDS    h;
    int     *cond;
    LBLBLK  *lblblk;
} CGOTO;

typedef struct {
    OPDS    h;
    MYFLT   *ndxvar, *incr, *limit;
    LBLBLK  *l;
} LOOP_OPS;

typedef struct {
    OPDS    h;
    MYFLT   *idel, *idur;
    LBLBLK  *lblblk;
    int32   cnt1, cnt2;
} TIMOUT;

typedef struct {
    OPDS    h;
} LINK;

typedef struct {
    OPDS    h;
    MYFLT  *inst;
} KILLOP;

/* the number of optional outputs defined in entry.c */
#define SUBINSTNUMOUTS  8

typedef struct {
    OPCODINFO *opcode_info;
    void    *uopcode_struct;
    INSDS   *parent_ip;
    MYFLT   *iobufp_ptrs[12];  /* expandable IV - Oct 26 2002 */ /* was 8 */
} OPCOD_IOBUFS;

typedef struct {                        /* IV - Oct 16 2002 */
    OPDS    h;
    MYFLT   *ar[VARGMAX];
    INSDS   *ip, *parent_ip;
    AUXCH   saved_spout;
    OPCOD_IOBUFS    buf;
} SUBINST;

typedef struct {                /* IV - Sep 8 2002: new structure: UOPCODE */
    OPDS          h;
    INSDS         *ip, *parent_ip;
    OPCOD_IOBUFS  *buf;
    /*unsigned int  l_ksmps;
    int           ksmps_scale;
    MYFLT         l_ekr, l_onedkr, l_onedksmps, l_kicvt;
    int           mode;*/
    /* special case: the argument list is stored at the end of the */
    /* opcode data structure */
    MYFLT         *ar[1];
} UOPCODE;

/* IV - Sep 8 2002: added opcodes: xin, xout, and setksmps */

typedef struct {
    OPDS    h;
    MYFLT   *args[1];
} XIN;

typedef struct {
    OPDS    h;
    MYFLT   *args[OPCODENUMOUTS_LOW];
} XIN_LOW;

typedef struct {
    OPDS    h;
    MYFLT   *args[OPCODENUMOUTS_HIGH];
} XIN_HIGH;

typedef struct {
    OPDS    h;
    MYFLT   *args[OPCODENUMOUTS_MAX];
} XIN_MAX;

typedef struct {
    OPDS    h;
    MYFLT   *args[1];
} XOUT;

typedef struct {
    OPDS    h;
    MYFLT   *args[OPCODENUMOUTS_LOW];
} XOUT_LOW;

typedef struct {
    OPDS    h;
    MYFLT   *args[OPCODENUMOUTS_HIGH];
} XOUT_HIGH;

typedef struct {
    OPDS    h;
    MYFLT   *args[OPCODENUMOUTS_MAX];
} XOUT_MAX;

typedef struct {
    OPDS    h;
    MYFLT   *i_ksmps;
} SETKSMPS;

typedef struct {                        /* IV - Oct 20 2002 */
    OPDS    h;
    MYFLT   *i_insno, *iname;
} NSTRNUM;

typedef struct {                        /* JPff Feb 2019 */
    OPDS    h;
    STRINGDAT *ans;
    MYFLT     *num;
} NSTRSTR;

typedef struct {
    OPDS    h;
    MYFLT   *kInsNo, *kFlags, *kRelease;
} TURNOFF2;

typedef struct {
    OPDS    h;
    MYFLT   *insno;
} DELETEIN;

void putop(CSOUND *csound, TEXT *tp);
void set_xtratim(CSOUND *csound, INSDS *ip);
void    schedofftim(CSOUND *, INSDS *);
void deact(CSOUND *csound, INSDS *ip);
void    timexpire(CSOUND *, double);
void print_amp_values(CSOUND *csound, int score_evt);
int insert(CSOUND *csound, int insno, EVTBLK *newevtp);
void infoff(CSOUND *csound, MYFLT p1);
int     MIDIinsert(CSOUND *, int, MCHNBLK*, MEVENT*);
void orcompact(CSOUND *csound);

int csoundKillInstanceInternal(CSOUND *csound, MYFLT instr, char *instrName,
                               int mode, int allow_release, int async);
uintptr_t event_insert_thread(void *p);

int     init0(CSOUND *);
void    xturnoff(CSOUND *, INSDS *);
void    xturnoff_now(CSOUND *, INSDS *);

int32_t prealloc(CSOUND *, AOP *);
int32_t prealloc_S(CSOUND *, AOP *);
int32_t kill_instance(CSOUND *csound, KILLOP *p);
int32_t subinstrset_S(CSOUND *, SUBINST *);
int32_t subinstrset(CSOUND *, SUBINST *), subinstr(CSOUND *, SUBINST *);
int32_t useropcdset(CSOUND *, UOPCODE *), useropcd(CSOUND *, UOPCODE *);
int32_t setksmpsset(CSOUND *, SETKSMPS *);
int32_t xinset(CSOUND *, XIN *), xoutset(CSOUND *, XOUT *);
int32_t nstrnumset(CSOUND *, NSTRNUM *);
int32_t nstrnumset_S(CSOUND *, NSTRNUM *), nstrstr(CSOUND *, NSTRSTR *);
int32_t delete_instr(CSOUND *, DELETEIN *);