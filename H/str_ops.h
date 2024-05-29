/*
    str_ops.h:

    Copyright (C) 2005, 2006 Istvan Varga
              (C) 2005       Matt J. Ingalls, John ffitch

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

#ifndef CSOUND_STR_OPS_H
#define CSOUND_STR_OPS_H

#include "csoundCore_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    OPDS    h;
    MYFLT   *indx;
    STRINGDAT  *str;
} STRSET_OP;

typedef struct {
    OPDS    h;
    MYFLT   *r;
    STRINGDAT  *str;
    char *mem;
} STRCHGD;

typedef struct {
    OPDS    h;
    MYFLT   *indx;
    MYFLT *str;
} STRTOD_OP;

typedef struct {
    OPDS    h;
    STRINGDAT   *r;
    MYFLT   *indx;
} STRGET_OP;

typedef struct {
    OPDS    h;
    STRINGDAT  *r;
    STRINGDAT   *str;
} STRCPY_OP;

typedef struct {
    OPDS    h;
    STRINGDAT   *r;
    STRINGDAT   *str1;
    STRINGDAT   *str2;
} STRCAT_OP;

typedef struct {
    OPDS    h;
    MYFLT   *r;
    STRINGDAT   *str1;
    STRINGDAT   *str2;
    MYFLT res;
} STRCMP_OP;

typedef struct {
    OPDS    h;
    STRINGDAT   *r;
    STRINGDAT   *sfmt;
    MYFLT   *args[64];
} SPRINTF_OP;

typedef struct {
    OPDS    h;
    STRINGDAT   *sfmt;
    MYFLT   *ktrig;
    MYFLT   *args[64];
    MYFLT   prv_ktrig;
} PRINTF_OP;

typedef struct {
    OPDS    h;
    STRINGDAT   *str;
    MYFLT   *ktrig;
    MYFLT   *no_newline;
    MYFLT   prv_ktrig;
    int     noNewLine;
} PUTS_OP;

typedef struct {
    OPDS    h;
    STRINGDAT   *Sdst;
    STRINGDAT   *Ssrc;
    MYFLT   *istart;
    MYFLT   *iend;
} STRSUB_OP;

typedef struct {
    OPDS    h;
    MYFLT   *ichr;
    STRINGDAT   *Ssrc;
    MYFLT   *ipos;
} STRCHAR_OP;

typedef struct {
    OPDS    h;
    MYFLT   *ilen;
    STRINGDAT   *Ssrc;
} STRLEN_OP;

typedef struct {
    OPDS    h;
    STRINGDAT   *Sdst;
    STRINGDAT   *Ssrc;
} STRUPPER_OP;

typedef struct {
    OPDS    h;
    STRINGDAT   *Sdst;
    MYFLT   *iopt;
} GETCFG_OP;

typedef struct {
    OPDS    h;
    MYFLT   *ipos;
    STRINGDAT   *Ssrc1;
    STRINGDAT   *Ssrc2;
} STRINDEX_OP;

typedef struct {
    OPDS    h;
    MYFLT   *inVar;
} PRINT_TYPE_OP;

void strset_option(CSOUND *csound, char *s);

int32_t strset_init(CSOUND *csound, STRSET_OP *p);
int32_t strget_init(CSOUND *csound, STRGET_OP *p);
int32_t strcpy_opcode_p(CSOUND *csound, STRGET_OP *p);
int32_t strcpy_opcode_S(CSOUND *csound, STRCPY_OP *p);
int32_t strassign_k(CSOUND *csound, STRCPY_OP *p);
int32_t strcat_opcode(CSOUND *csound, STRCAT_OP *p);
int32_t strcmp_opcode(CSOUND *csound, STRCMP_OP *p);
int32_t sprintf_opcode(CSOUND *csound, SPRINTF_OP *p);
int32_t printf_opcode_init(CSOUND *csound, PRINTF_OP *p);
int32_t printf_opcode_set(CSOUND *csound, PRINTF_OP *p);
int32_t printf_opcode_perf(CSOUND *csound, PRINTF_OP *p);
int32_t puts_opcode_init(CSOUND *csound, PUTS_OP *p);
int32_t puts_opcode_perf(CSOUND *csound, PUTS_OP *p);
int32_t strtod_opcode_p(CSOUND *csound, STRTOD_OP *p);
int32_t strtod_opcode_S(CSOUND *csound, STRSET_OP *p);
int32_t strtol_opcode_p(CSOUND *csound, STRTOD_OP *p);
int32_t strtol_opcode_S(CSOUND *csound, STRSET_OP *p);
int32_t strsub_opcode(CSOUND *csound, STRSUB_OP *p);
int32_t strchar_opcode(CSOUND *csound, STRCHAR_OP *p);
int32_t strlen_opcode(CSOUND *csound, STRLEN_OP *p);
int32_t strupper_opcode(CSOUND *csound, STRUPPER_OP *p);
int32_t strlower_opcode(CSOUND *csound, STRUPPER_OP *p);
int32_t getcfg_opcode(CSOUND *csound, GETCFG_OP *p);
int32_t strindex_opcode(CSOUND *csound, STRINDEX_OP *p);
int32_t strrindex_opcode(CSOUND *csound, STRINDEX_OP *p);
int     str_changed(CSOUND *csound, STRCHGD *p);
int     str_changed_k(CSOUND *csound, STRCHGD *p);
#ifdef HAVE_CURL
int     str_from_url(CSOUND *csound, STRCPY_OP *p);
#endif
int32_t print_type_opcode(CSOUND* csound, PRINT_TYPE_OP* p);
int32_t s_opcode(CSOUND *csound, STRGET_OP *p);
int32_t s_opcode_k(CSOUND *csound, STRGET_OP *p);

#ifdef __cplusplus
}
#endif

#endif      /* CSOUND_STR_OPS_H */
