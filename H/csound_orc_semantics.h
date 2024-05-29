/*
    csound_orc_sematics.h:

    Copyright (C) 2013 by Steve Yi

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

#ifndef CSOUND_ORC_SEMANTICS_H
#define CSOUND_ORC_SEMANTICS_H

#include "csoundCore_internal.h"
#include "csound_orc.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char *SYNTHESIZED_ARG;

/** Gets short version of opcode name, trimming off anything after '.'.
 If opname has no '.' in name, simply returns the opname pointer.
 If the name is truncated, caller is responsible for calling mfree
 on returned value.  Caller should compare the returned value with the
 passed in opname to see if it is different and thus requires mfree'ing. */

char* get_arg_type2(CSOUND* csound, TREE* tree, TYPE_TABLE* typeTable);

typedef struct csstructvar {
  CS_VAR_MEM** members;
} CS_STRUCT_VAR;

extern const char* UNARY_PLUS;

void do_baktrace(CSOUND *, uint64_t);

void delete_tree(CSOUND *csound, TREE *l);

OENTRIES* find_opcode2(CSOUND* csound, char* opname);

/* Given an OENTRIES list, resolve to a single OENTRY* based on the
 * found in- and out- argtypes.  Returns NULL if opcode could not be
 * resolved. If more than one entry matches, mechanism assumes there
 * are multiple opcode entries with same types and last one should
 * override previous definitions.
 */
OENTRY* resolve_opcode(CSOUND* csound, OENTRIES* entries,
                       char* outArgTypes, char* inArgTypes);

TREE* make_node(CSOUND *, int, int, int, TREE*, TREE*);
TREE* make_leaf(CSOUND *, int, int, int, ORCTOKEN*);
TREE* copy_node(CSOUND*, TREE*);

char    *cs_strndup(CSOUND*, char*, size_t);
void print_tree(CSOUND *, char *, TREE *);
void handle_optional_args(CSOUND *csound, TREE *l);
char* resolve_opcode_get_outarg(CSOUND* csound,
                                OENTRIES* entries, char* inArgTypes);
TREE* appendToTree(CSOUND * csound, TREE *first, TREE *newlast);
char* get_arg_string_from_tree(CSOUND* csound, TREE* tree,
                               TYPE_TABLE* typeTable);
void add_arg(CSOUND* csound, char* varName, char* annotation, TYPE_TABLE* typeTable);
void add_array_arg(CSOUND* csound, char* varName, char* annotation, int dimensions,
                   TYPE_TABLE* typeTable);
char* get_array_sub_type(CSOUND* csound, char* arrayName);
char* convert_external_to_internal(CSOUND* csound, char* arg);
TREE* verify_tree(CSOUND * csound, TREE *root, TYPE_TABLE* typeTable);
int check_in_arg(char* found, char* required);
int check_in_args(CSOUND* csound, char* inArgsFound, char* opInArgs);
int check_out_arg(char* found, char* required);
int check_out_args(CSOUND* csound, char* outArgsFound, char* opOutArgs);
void csound_orcerror(PARSE_PARM *pp, void *yyscanner,
                     CSOUND *csound, TREE **astTree, const char *str);
TREE* make_opcall_from_func_start(CSOUND *csound, int line, int locn, int type,
                                  TREE* left, TREE* right);

#ifdef __cplusplus
}
#endif

#endif
