/*
    aops.h:

    Copyright (C) 1991 Barry Vercoe, John ffitch, Gabriel Maldonado

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

/*                                                      AOPS.H          */

#pragma once

#include "csoundCore_internal.h"

#define CSOUND_SPIN_SPINLOCK csoundSpinLock(&csound->spinlock);
#define CSOUND_SPIN_SPINUNLOCK csoundSpinUnLock(&csound->spinlock);
#define CSOUND_SPOUT_SPINLOCK csoundSpinLock(&csound->spoutlock);
#define CSOUND_SPOUT_SPINUNLOCK csoundSpinUnLock(&csound->spoutlock);

#if ULONG_MAX == 18446744073709551615UL
#  define POW2MAX   (24.0)
#else
#  define POW2MAX   (15.0)
#endif

typedef struct {
    OPDS    h;
    MYFLT   *r, *a;
} ASSIGN;

#define ASSIGNM_MAX (24)
typedef struct {
    OPDS    h;
    MYFLT   *r[ASSIGNM_MAX], *a[ASSIGNM_MAX];
} ASSIGNM;

typedef struct {
    OPDS    h;
    TABDAT  *a;
    MYFLT   *size, *value;
} INITT;

typedef struct {
    OPDS    h;
    TABDAT  *tab;
    MYFLT   *ind;
    MYFLT   *val;
} ASSIGNT;

typedef struct {
    OPDS    h;
    MYFLT   *ans;
    TABDAT  *tab;
    MYFLT   *ind;
} TABREF;


typedef struct {
    OPDS    h;
    int32_t     *rbool;
    MYFLT   *a, *b;
} RELAT;

typedef struct {
    OPDS    h;
    int32_t     *rbool, *ibool, *jbool;
} LOGCL;

typedef struct {
    OPDS    h;
    MYFLT   *r;
    int32_t     *cond;
    MYFLT   *a, *b;
} CONVAL;

typedef struct {
    OPDS    h;
    MYFLT   *r, *a, *b;
} AOP;

typedef struct {
    OPDS    h;
    MYFLT   *r, *a, *b, *def;
} DIVZ;

typedef struct {
    OPDS    h;
    MYFLT   *r, *a;
} EVAL;

typedef struct {
    OPDS    h;
    MYFLT   *ar;
} INM;

typedef struct {
    OPDS    h;
    ARRAYDAT   *tabout;
} INA;

typedef struct {
    OPDS    h;
    MYFLT   *ar1, *ar2;
} INS;

typedef struct {
    OPDS    h;
    MYFLT   *ar1, *ar2, *ar3, *ar4;
} INQ;

typedef struct {
    OPDS    h;
    MYFLT   *ar1, *ar2, *ar3, *ar4, *ar5, *ar6;
} INH;

typedef struct {
    OPDS    h;
    MYFLT   *ar1, *ar2, *ar3, *ar4, *ar5, *ar6, *ar7, *ar8;
} INO;

typedef struct {
    OPDS    h;
    MYFLT   *ar[40];    /* array size should be consistent with entry2.c */
} INALL;

typedef struct {
    OPDS    h;
    MYFLT   *ar[40];
    MYFLT   *ch[VARGMAX];
    int32_t     init;
} INCH;

typedef struct {
    OPDS    h;
    MYFLT   *ar;
    MYFLT   *ch;
    int32_t     init;
} INCH1;

typedef struct {
    OPDS    h;
    MYFLT   *asig[VARGMAX];
} OUTX;

typedef struct {
    OPDS       h;
    ARRAYDAT   *tabin;
    int32_t    nowarn;
} OUTARRAY;

typedef struct {
    OPDS    h;
    MYFLT   *asig;
} OUTM;

typedef struct {
    OPDS    h;
    MYFLT   *args[VARGMAX];
} OUTCH;

typedef struct {
    OPDS    h;
    MYFLT   *r, *pc, *et, *cy, *ref;
} XENH;

typedef struct {
    OPDS    h;
    MYFLT   *r, *ktrig, *kinput, *tablenum;
    MYFLT   old_r;
} CPSTUN;

typedef struct {
    OPDS    h;
    MYFLT   *r, *input, *tablenum;
} CPSTUNI;

typedef struct {
    OPDS    h;
    MYFLT   *res, *arg;
} ERRFN;

typedef struct MONITOR_OPCODE_ {
    OPDS    h;
    MYFLT   *ar[24];
} MONITOR_OPCODE;

typedef struct {
        OPDS    h;
        MYFLT   *kstartChan, *argums[VARGMAX];
        int32_t narg;
} OUTRANGE;

MYFLT MOD(MYFLT a, MYFLT bb);

int32_t monitor_opcode_perf(CSOUND *csound, MONITOR_OPCODE *p);
int32_t monitor_opcode_init(CSOUND *csound, MONITOR_OPCODE *p);
int32_t outRange_i(CSOUND *csound, OUTRANGE *p);
int32_t outRange(CSOUND *csound, OUTRANGE *p);
int32_t hw_channels(CSOUND *csound, ASSIGN *p);
void csound_aops_init_tables(CSOUND *csound);

int32_t gaassign(CSOUND *, ASSIGN *), rassign(CSOUND *, ASSIGN *);
int32_t aassign(CSOUND *, ASSIGN *, int32_t islocal), laassign(CSOUND *, ASSIGN *);
int32_t assign(CSOUND *, ASSIGN *);
int32_t ainit(CSOUND *, ASSIGN *);
int32_t minit(CSOUND *, ASSIGNM *), mainit(CSOUND *, ASSIGNM *);
/* int32_t tinit(CSOUND *, void *), tassign(CSOUND *, void *); */
/* int32_t tabref_check(CSOUND *, void *), tabref(CSOUND *, void *); */
int32_t gt(CSOUND *, RELAT *), ge(CSOUND *, RELAT *);
int32_t lt(CSOUND *, RELAT *), le(CSOUND *, RELAT *);
int32_t eq(CSOUND *, RELAT *), ne(CSOUND *, RELAT *);
int32_t and(CSOUND *, LOGCL *), or(CSOUND *, LOGCL *);
int32_t b_not(CSOUND *, LOGCL*);
int32_t conval(CSOUND *, CONVAL *), aconval(CSOUND *, CONVAL *);
int32_t addkk(CSOUND *, AOP *), subkk(CSOUND *, AOP *);
int32_t mulkk(CSOUND *, AOP *), divkk(CSOUND *, AOP *);
int32_t modkk(CSOUND *, AOP *);
int32_t addka(CSOUND *, AOP *), subka(CSOUND *, AOP *);
int32_t mulka(CSOUND *, AOP *), divka(CSOUND *, AOP *);
int32_t modka(CSOUND *, AOP *);
int32_t addak(CSOUND *, AOP *), subak(CSOUND *, AOP *);
int32_t mulak(CSOUND *, AOP *), divak(CSOUND *, AOP *);
int32_t modak(CSOUND *, AOP *);
int32_t addaa(CSOUND *, AOP *), subaa(CSOUND *, AOP *);
int32_t mulaa(CSOUND *, AOP *), divaa(CSOUND *, AOP *);
int32_t modaa(CSOUND *, AOP *);
int32_t addin(CSOUND *, ASSIGN *), addina(CSOUND *, ASSIGN *);
int32_t subin(CSOUND *, ASSIGN *), subina(CSOUND *, ASSIGN *);
int32_t addinak(CSOUND *, ASSIGN *), subinak(CSOUND *, ASSIGN *);
int32_t divzkk(CSOUND *, DIVZ *), divzka(CSOUND *, DIVZ *);
int32_t divzak(CSOUND *, DIVZ *), divzaa(CSOUND *, DIVZ *);
int32_t int1(CSOUND *, EVAL *), int1a(CSOUND *, EVAL *);
int32_t frac1(CSOUND *, EVAL *), frac1a(CSOUND *, EVAL *);
int32_t int1_round(CSOUND *, EVAL *), int1a_round(CSOUND *, EVAL *);
int32_t int1_floor(CSOUND *, EVAL *), int1a_floor(CSOUND *, EVAL *);
int32_t int1_ceil(CSOUND *, EVAL *), int1a_ceil(CSOUND *, EVAL *);
int32_t rnd1(CSOUND *, EVAL *), birnd1(CSOUND *, EVAL *);
int32_t rnd1seed(CSOUND *, INM *);
int32_t abs1(CSOUND *, EVAL *), exp01(CSOUND *, EVAL *);
int32_t log01(CSOUND *, EVAL *), sqrt1(CSOUND *, EVAL *);
int32_t sin1(CSOUND *, EVAL *), cos1(CSOUND *, EVAL *);
int32_t is_NaN(CSOUND *, ASSIGN *), is_NaNa(CSOUND *, ASSIGN *);
int32_t is_inf(CSOUND *, ASSIGN *), is_infa(CSOUND *, ASSIGN *);
int32_t tan1(CSOUND *, EVAL *), asin1(CSOUND *, EVAL *);
int32_t acos1(CSOUND *, EVAL *), atan1(CSOUND *, EVAL *);
int32_t sinh1(CSOUND *, EVAL *), cosh1(CSOUND *, EVAL *);
int32_t tanh1(CSOUND *, EVAL *), log101(CSOUND *, EVAL *), log21(CSOUND *, EVAL *);
int32_t atan21(CSOUND *, AOP *), atan2aa(CSOUND *, AOP *);
int32_t absa(CSOUND *, EVAL *), expa(CSOUND *, EVAL *);
int32_t loga(CSOUND *, EVAL *), sqrta(CSOUND *, EVAL *);
int32_t sina(CSOUND *, EVAL *), cosa(CSOUND *, EVAL *);
int32_t tana(CSOUND *, EVAL *), asina(CSOUND *, EVAL *);
int32_t acosa(CSOUND *, EVAL *), atana(CSOUND *, EVAL *);
int32_t sinha(CSOUND *, EVAL *), cosha(CSOUND *, EVAL *);
int32_t tanha(CSOUND *, EVAL *), log10a(CSOUND *, EVAL *), log2a(CSOUND *, EVAL *);
int32_t dbamp(CSOUND *, EVAL *), ampdb(CSOUND *, EVAL *);
int32_t aampdb(CSOUND *, EVAL *), dbfsamp(CSOUND *, EVAL *);
int32_t ampdbfs(CSOUND *, EVAL *), aampdbfs(CSOUND *, EVAL *);
int32_t ftlen(CSOUND *, EVAL *), ftlptim(CSOUND *, EVAL *);
int32_t ftchnls(CSOUND *, EVAL *), ftcps(CSOUND *, EVAL *);
int32_t signum(CSOUND *, ASSIGN *), asignum(CSOUND *, ASSIGN *);
int32_t rtclock(CSOUND *, EVAL *);
int32_t cpsoct(CSOUND *, EVAL *), octpch(CSOUND *, EVAL *);
int32_t cpspch(CSOUND *, EVAL *), pchoct(CSOUND *, EVAL *);
int32_t octcps(CSOUND *, EVAL *), acpsoct(CSOUND *, EVAL *);
int32_t cpsmidinn(CSOUND *, EVAL *), octmidinn(CSOUND *, EVAL *);
int32_t pchmidinn(CSOUND *, EVAL *);
int32_t inarray(CSOUND *, INA *);
int32_t in(CSOUND *, INM *), ins(CSOUND *, INS *);
int32_t inq(CSOUND *, INQ *), inh(CSOUND *, INH *);
int32_t ino(CSOUND *, INO *), in16(CSOUND *, INALL *);
int32_t in32(CSOUND *, INALL *), outarr_init(CSOUND *, OUTARRAY *);
int32_t outarr(CSOUND *, OUTARRAY *), outrep(CSOUND*, OUTM*);
int32_t inch_opcode(CSOUND *, INCH *), inall_opcode(CSOUND *, INALL *);
int32_t inch_set(CSOUND*, INCH*);
/* int32_t out(CSOUND *, void *), outs(CSOUND *, void *); */
int32_t outs1(CSOUND *, OUTM *), outs2(CSOUND *, OUTM *);
int32_t och2(CSOUND *, OUTM *), och3(CSOUND *, OUTM *);
int32_t och4(CSOUND *, OUTM *), ochn(CSOUND *, OUTX *);
int32_t outq3(CSOUND *, OUTM *), outq4(CSOUND *, OUTM *);
/* int32_t outh(CSOUND *, void *), outo(CSOUND *, void *); */
/* int32_t outx(CSOUND *, void *), outX(CSOUND *, void *); */
int32_t outch(CSOUND *, OUTCH *), outall(CSOUND *, OUTX *);
int32_t numsamp(CSOUND *, EVAL *), ftsr(CSOUND *, EVAL *);
int32_t ilogbasetwo(CSOUND *, EVAL *), logbasetwo_set(CSOUND *, EVAL *);
int32_t powoftwo(CSOUND *, EVAL *), powoftwoa(CSOUND *, EVAL *);
int32_t logbasetwo(CSOUND *, EVAL *), logbasetwoa(CSOUND *, EVAL *);
int32_t cpsxpch(CSOUND *, XENH *), cps2pch(CSOUND *, XENH *);
int32_t cpstun(CSOUND *, CPSTUN *), cpstun_i(CSOUND *, CPSTUNI *);
int32_t db(CSOUND *, EVAL *), dba(CSOUND *, EVAL *);
int32_t semitone(CSOUND *, EVAL *), asemitone(CSOUND *, EVAL *);
int32_t cent(CSOUND *, EVAL *), acent(CSOUND *, EVAL *);
int32_t error_fn(CSOUND *csound, ERRFN *p);
int32_t inch1_set(CSOUND *csound, INCH1 *p);
int32_t inch_opcode1(CSOUND *csound, INCH1 *p);