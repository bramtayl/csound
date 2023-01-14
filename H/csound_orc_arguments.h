/*
 csound_orc_arguments.h:

 Copyright (C) 2023

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


#ifndef CSOUND_ORC_ARGUMENTS_H
#define CSOUND_ORC_ARGUMENTS_H 1

#include "csound_data_structures.h"

typedef struct csorcarg {
    char*    text;        // human readable text
    char*    exprIdent;   // name with which to find the expression result in pools
    CS_TYPE* cstype;
    int      type;        // token-type
    int      isGlobal;
    int      isPfield;
    int      isExpression;
    int      dimensions;  // 0 scalar, 1 or more means n-dimenional array
    int      dimension;  // ex. ix[0] is dimension 1 of 1 dimensional array
    int      linenum;
    struct csorcargs* SubExpression; // when the argument itself is an expr
} CSOUND_ORC_ARGUMENT;

typedef struct csorcargs {
    CONS_CELL* list;
    int        length;
    struct csorcarg* (*nth)(struct csorcargs*, int);
    // struct csorcarg* (*next)(struct csorcargs*);
    void             (*append)(CSOUND*, struct csorcargs*, struct csorcarg*);
} CSOUND_ORC_ARGUMENTS;

CSOUND_ORC_ARGUMENTS* new_csound_orc_arguments(CSOUND*);

#endif