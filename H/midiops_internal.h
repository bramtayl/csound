#pragma once

#include "midiops.h"
#include "csound.h"

int32_t massign_S(CSOUND *, MASSIGNS *);
int32_t massign_p(CSOUND *, MASSIGN *), ctrlinit(CSOUND *, CTLINIT *);
int32_t ctrlnameinit(CSOUND *, CTLINITS *);
int32_t notnum(CSOUND *, MIDIKMB *), veloc(CSOUND *, MIDIMAP *);
int32_t pchmidi(CSOUND *, MIDIKMB *), pchmidib(CSOUND *, MIDIKMB *);
int32_t octmidi(CSOUND *, MIDIKMB *), octmidib(CSOUND *, MIDIKMB *);
int32_t pchmidib_i(CSOUND *, MIDIKMB *), octmidib_i(CSOUND *, MIDIKMB *);
int32_t icpsmidib_i(CSOUND *, MIDIKMB *), cpsmidi(CSOUND *, MIDIKMB *);
int32_t icpsmidib(CSOUND *, MIDIKMB *), kcpsmidib(CSOUND *, MIDIKMB *);
int32_t midibset(CSOUND *, MIDIKMB *);
int32_t ampmidi(CSOUND *, MIDIAMP *);
int32_t aftset(CSOUND *, MIDIKMAP *), aftouch(CSOUND *, MIDIKMAP *);
int32_t ipchbend(CSOUND *, MIDIMAP *);
int32_t kbndset(CSOUND *, MIDIKMAP *), kpchbend(CSOUND *, MIDIKMAP *);
int32_t imidictl(CSOUND *, MIDICTL *), mctlset(CSOUND *, MIDICTL *);
int32_t midictl(CSOUND *, MIDICTL *), imidiaft(CSOUND *, MIDICTL *);
int32_t maftset(CSOUND *, MIDICTL *), midiaft(CSOUND *, MIDICTL *);
int32_t ichanctl(CSOUND *, CHANCTL *), chctlset(CSOUND *, CHANCTL *);
int32_t chanctl(CSOUND *, CHANCTL *);
int32_t cpstmid(CSOUND *, CPSTABLE *);
int32_t midichn(CSOUND *, MIDICHN *), pgmassign(CSOUND *, PGMASSIGN *),
        pgmassign_S(CSOUND *, PGMASSIGN *);
int32_t midiin_set(CSOUND *, MIDIIN *), midiin(CSOUND *, MIDIIN *);
int32_t pgmin_set(CSOUND *, PGMIN *), pgmin(CSOUND *, PGMIN *);
int32_t ctlin_set(CSOUND *, CTLIN *); 
int32_t ctlin(CSOUND *, CTLIN *);
int32_t savectrl_init(CSOUND*, SAVECTRL*), savectrl_perf(CSOUND*, SAVECTRL*);
int32_t printctrl(CSOUND*, PRINTCTRL*);
int32_t printctrl_init(CSOUND*, PRINTCTRL*), printctrl_init1(CSOUND*, PRINTCTRL*);
int32_t presetctrl_init(CSOUND*, PRESETCTRL*), presetctrl_perf(CSOUND*, PRESETCTRL*);
int32_t presetctrl1_init(CSOUND*, PRESETCTRL1*), presetctrl1_perf(CSOUND*, PRESETCTRL1*);
int32_t selectctrl_init(CSOUND*, SELECTCTRL*), selectctrl_perf(CSOUND*, SELECTCTRL*);
int32_t printpresets_init(CSOUND*, PRINTPRESETS*), printpresets_init1(CSOUND*, PRINTPRESETS*);
int32_t printpresets_perf(CSOUND*, PRINTPRESETS*);
int32_t midiarp_set(CSOUND *, MIDIARP *);
int32_t midiarp(CSOUND *, MIDIARP *);