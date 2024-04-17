/*
 csound_type_system.c:

 Copyright (C) 2012, 2013 Steven Yi

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

#ifndef CSOUND_TYPE_SYSTEM_H
#define CSOUND_TYPE_SYSTEM_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "csound.h"
#include "csound_data_structures.h"
#include <stdint.h>

#define CS_ARG_TYPE_BOTH 0
#define CS_ARG_TYPE_IN 1
#define CS_ARG_TYPE_OUT 2

#if defined(UINTPTR_MAX) && defined(UINT64_MAX) && (UINTPTR_MAX == UINT64_MAX)
#define CS_VAR_TYPE_OFFSET (sizeof(CS_VAR_MEM) - sizeof(double))
#else
#define CS_VAR_TYPE_OFFSET (sizeof(CS_VAR_MEM) - sizeof(MYFLT))
#endif

    typedef struct cstypeitem {
      CS_TYPE* cstype;
      struct cstypeitem* next;
    } CS_TYPE_ITEM;

    typedef struct typepool {
        CS_TYPE_ITEM* head;
    } TYPE_POOL;

    /* Adds a new type to Csound's type table
       Returns if variable type redefined */
    PUBLIC int csoundAddVariableType(CSOUND* csound, TYPE_POOL* pool,
                                     CS_TYPE* typeInstance);
    PUBLIC CS_VARIABLE* csoundCreateVariable(CSOUND* csound, TYPE_POOL* pool,
                                             CS_TYPE* type, char* name,
                                             void* typeArg);
    PUBLIC CS_TYPE* csoundGetTypeWithVarTypeName(TYPE_POOL* pool, char* typeName);

    PUBLIC CS_VAR_POOL* csoundCreateVarPool(CSOUND* csound);
    PUBLIC void csoundFreeVarPool(CSOUND* csound, CS_VAR_POOL* pool);
    PUBLIC char* getVarSimpleName(CSOUND* csound, const char* name);
    PUBLIC CS_VARIABLE* csoundFindVariableWithName(CSOUND* csound,
                                                   CS_VAR_POOL* pool,
                                                   const char* name);
    PUBLIC int csoundAddVariable(CSOUND* csound, CS_VAR_POOL* pool,
                                 CS_VARIABLE* var);
    PUBLIC void recalculateVarPoolMemory(CSOUND* csound, CS_VAR_POOL* pool);
    PUBLIC void reallocateVarPoolMemory(CSOUND* csound, CS_VAR_POOL* pool);
    PUBLIC void initializeVarPool(CSOUND* csound, MYFLT* memBlock, CS_VAR_POOL* pool);

#ifdef  __cplusplus
}
#endif

#endif  /* CSOUND_TYPE_SYSTEM_H */
