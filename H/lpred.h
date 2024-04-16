/*
  lpred.h:

  Copyright 2020 Victor Lazzarini

  streaming linear prediction

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

#ifndef CSOUND_LPRED_H
#define CSOUND_LPRED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pstream.h"

  typedef struct _mycmplx {
    MYFLT re; MYFLT im;
  } MYCMPLX;


  typedef struct _lpfil {
    OPDS h;
    MYFLT *out;
    MYFLT *in, *koff, *kflag, *ifn, *isiz, *iord, *iwin;
    AUXCH coefs;
    AUXCH del;
    AUXCH buf;
    int32_t M, N, wlen;
    int32_t rp;
    void *setup;
    MYFLT *win, g;
    FUNC *ft;
  } LPCFIL;

  typedef struct _lpfil2 {
    OPDS h;
    MYFLT *out;
    MYFLT *in, *sig, *flag, *prd, *isiz, *iord, *iwin;
    AUXCH coefs;
    AUXCH del;
    AUXCH buf;
    AUXCH cbuf;
    int32_t M, N, wlen;
    int32_t rp,bp,cp;
    MYFLT *win, g;
    void *setup;
  } LPCFIL2;

  typedef struct _lpreda {
    OPDS h;
    ARRAYDAT *out;
    MYFLT *rms, *err, *cps;
    MYFLT  *off, *flag, *ifn, *isiz, *iord, *iwin;
    AUXCH buf;
    int32_t M, N, wlen;
    FUNC *ft;
    MYFLT *win;
    void *setup;
  } LPREDA;

  typedef struct _lpfil3 {
    OPDS h;
    MYFLT *out, *in;
    ARRAYDAT *coefs;
    AUXCH del;
    int32_t M;
    int32_t rp;
    void *setup;
  } LPCFIL3;

  typedef struct _lpreda2 {
    OPDS h;
    ARRAYDAT *out;
    MYFLT *rms, *err, *cps;
    MYFLT  *in, *flag, *prd, *isiz, *iord, *iwin;
    AUXCH cbuf;
    AUXCH buf;
    int32_t M, N, wlen, cp, bp;
    MYFLT *win;
    void *setup;
  } LPREDA2;


  typedef struct _lpreda3 {
    OPDS h;
    PVSDAT *fout;
    MYFLT  *in, *isiz, *prd, *iord, *iwin;
    AUXCH cbuf;
    AUXCH buf;
    AUXCH fftframe;
    int32_t M, N, wlen, cp, bp;
    MYFLT *win;
    void *setup;
  } LPCPVS;


  typedef struct _pvscoefs {
    OPDS h;
    ARRAYDAT *out;
    MYFLT *krms, *kerr;
    PVSDAT  *fin;
    MYFLT  *iord, *imod;
    AUXCH coef;
    AUXCH buf;
    int32_t M, N;
    MYFLT rms;
    MYFLT err;
    MYFLT mod;
    uint32_t framecount;
    void *setup;
  } PVSCFS;


  typedef struct _cf2p {
    OPDS h;
    ARRAYDAT *out;
    ARRAYDAT *in;
    int32_t M;
    void *setup;
    MYFLT sum;
  } CF2P;

  typedef struct {
    OPDS    h;
    MYFLT   *ar, *asig;
    ARRAYDAT *kparm;
    MYFLT   *kmin, *kmax, *iprd, *imod, *iscl, *istor;
    int     scale, ord;
    AUXCH   y1m,y2m,y1o,y2o,y1c,y2c;
    MYFLT kcnt;
  } RESONB;


#ifdef __cplusplus
}
#endif

#endif      /* CSOUND_LPRED_H */
