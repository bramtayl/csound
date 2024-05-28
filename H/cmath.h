/*
    cmath.h:

    Copyright (C) 1994 Paris Smaragdis, John ffitch

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

#include "sysdep.h"
#include "csoundCore_common.h"

/* returns 0 on success, -1 if there are insufficient arguments, */
/* and -2 in the case of an unknown distribution */
int32_t gen21_rand(FGDATA *ff, FUNC *ftp);

typedef struct  {
        OPDS    h;
        MYFLT   *sr, *in, *powerOf, *norm;
 } POW;

typedef struct  {
        OPDS    h;
        MYFLT   *out, *arg1, *arg2, *arg3;
} PRAND;

typedef struct  {
        OPDS    h;
        MYFLT   *ar, *arg1, *xamp, *xcps;
        MYFLT   *iseed;
        MYFLT   dfdmax, num1, num2;
        int32_t   phs;
        int32_t     ampcod, cpscod;
} PRANDI;

typedef struct {
        OPDS   h;
        MYFLT  *ans;
} GETSEED;

typedef struct gauss{
  OPDS h;
  MYFLT *a, *mu, *sigma;
  MYFLT z;
  int flag;
} GAUSS;

int32_t ipow(CSOUND *, POW *), apow(CSOUND *, POW *);
int32_t alinear(CSOUND *, PRAND *), iklinear(CSOUND *, PRAND *);
int32_t atrian(CSOUND *, PRAND *), iktrian(CSOUND *, PRAND *);
int32_t aexp(CSOUND *, PRAND *), ikexp(CSOUND *, PRAND *);
int32_t abiexp(CSOUND *, PRAND *), ikbiexp(CSOUND *, PRAND *);
int32_t agaus(CSOUND *, PRAND *), ikgaus(CSOUND *, PRAND *);
int32_t acauchy(CSOUND *, PRAND *), ikcauchy(CSOUND *, PRAND *);
int32_t apcauchy(CSOUND *, PRAND *), ikpcauchy(CSOUND *, PRAND *);
int32_t abeta(CSOUND *, PRAND *), ikbeta(CSOUND *, PRAND *);
int32_t aweib(CSOUND *, PRAND *), ikweib(CSOUND *, PRAND *);
int32_t apoiss(CSOUND *, PRAND *), ikpoiss(CSOUND *, PRAND *);
int32_t seedrand(CSOUND *, PRAND *), getseed(CSOUND *, GETSEED *);
int32_t auniform(CSOUND *, PRAND *), ikuniform(CSOUND *, PRAND *);
int32_t iexprndi(CSOUND *, PRANDI *), exprndiset(CSOUND *, PRANDI *);
int32_t kexprndi(CSOUND *, PRANDI *), aexprndi(CSOUND *, PRANDI *);
int32_t icauchyi(CSOUND *, PRANDI *), cauchyiset(CSOUND *, PRANDI *);
int32_t kcauchyi(CSOUND *, PRANDI *), acauchyi(CSOUND *, PRANDI *);
int32_t igaussi(CSOUND *, PRANDI *), gaussiset(CSOUND *, PRANDI *);
int32_t kgaussi(CSOUND *, PRANDI *), agaussi(CSOUND *, PRANDI *);
int32_t gauss_scalar(CSOUND *csound, GAUSS *p);
int32_t gauss_vector(CSOUND *csound, GAUSS *p);