#pragma once

#include "csound.h"
#include "insert.h"
#include "aops.h"

int32_t ihold(CSOUND *, LINK *);
int32_t turnoff(CSOUND *, LINK *);
int32_t igoto(CSOUND *, GOTO *), kgoto(CSOUND *, GOTO *);
int32_t icgoto(CSOUND *, CGOTO *), kcgoto(CSOUND *, CGOTO *);
int32_t timset(CSOUND *, TIMOUT *), timout(CSOUND *, TIMOUT *);
int32_t reinit(CSOUND *, GOTO *), rigoto(CSOUND *, GOTO *);
int32_t rireturn(CSOUND *, LINK *), tigoto(CSOUND *, GOTO *);
int32_t tival(CSOUND *, EVAL *);
int32_t ingoto(CSOUND *, CGOTO *), kngoto(CSOUND *, CGOTO *);
int32_t turnoff2k(CSOUND *, TURNOFF2 *);
int32_t turnoff2S(CSOUND *, TURNOFF2 *) ;
int32_t turnoff3S(CSOUND *, TURNOFF2 *), turnoff3k(CSOUND *, TURNOFF2 *);
int32_t loop_l_i(CSOUND *, LOOP_OPS *), loop_le_i(CSOUND *, LOOP_OPS *);
int32_t loop_g_i(CSOUND *, LOOP_OPS *), loop_ge_i(CSOUND *, LOOP_OPS *);
int32_t loop_l_p(CSOUND *, LOOP_OPS *), loop_le_p(CSOUND *, LOOP_OPS *);
int32_t loop_g_p(CSOUND *, LOOP_OPS *), loop_ge_p(CSOUND *, LOOP_OPS *);