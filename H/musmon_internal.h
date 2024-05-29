#pragma once

#include "csound.h"
#include "oload.h"
#include "csoundCore_internal.h"
#include "midiops.h"

#define STA_MUSMON(x)   (csound->musmonStatics.x)

#define STRING_HASH(arg) STRSH(arg)
#define STRSH(arg) #arg

typedef struct evt_cb_func {
  void    (*func)(CSOUND *, void *);
  void    *userData;
  struct evt_cb_func  *nxt;
} EVT_CB_FUNC;

void settempo(CSOUND *csound, double tempo);
int     musmon(CSOUND *);
void delete_selected_rt_events(CSOUND *csound, MYFLT instr);
int lplay(CSOUND *csound, EVLIST *a);    /* cscore re-entry into musmon */
int sensevents(CSOUND *csound);
void deactivate_all_notes(CSOUND *csound);
void beatexpire(CSOUND *csound, double beat);
int process_score_event(CSOUND *csound, EVTBLK *evt, int rtEvt);
void section_amps(CSOUND *csound, int enable_msgs);
void print_maxamp(CSOUND *csound, MYFLT x);
void delete_pending_rt_events(CSOUND *csound);
void musmon_rewind_score(CSOUND *csound);
void print_csound_version(CSOUND* csound);

OPCODE_INIT_FUNCTION(musmon_internal_localops_init);

