#pragma once

#include "csound.h"
#include "parse_param.h"

ORCTOKEN* make_int(CSOUND *,char *);
ORCTOKEN* make_num(CSOUND *,char *);
ORCTOKEN *make_token(CSOUND *csound, char *s);
ORCTOKEN *make_label(CSOUND *, char *s);
ORCTOKEN *make_string(CSOUND *, char *);
ORCTOKEN *new_token(CSOUND *, int);

char *csound_orcget_current_pointer(void *);
uint64_t csound_orcget_iline(void *);
uint64_t csound_orcget_ilocn(void *);
int csound_orclex(ORCTOKEN**, CSOUND *, void *);
int csound_orclex_init(void **);
int csound_orcget_lineno(void*);
char *csound_orcget_text( void *scanner );
uint64_t csound_orcget_locn(void *);
int csound_orclex_destroy(void *);
int csound_orcparse(PARSE_PARM *, void *, CSOUND*, TREE**);
void csound_orcput_ilocn(void *, uint64_t, uint64_t);
void csound_orcset_extra(PARSE_PARM *, void *);
void csound_orcset_lineno(int, void*);
TREE *csound_orc_expand_expressions(CSOUND *, TREE *);