#pragma once

#include "csound.h"

void m_chn_init_all(CSOUND *csound);
void MidiOpen(CSOUND *csound);
int32_t m_chinsno(CSOUND *csound, int32_t chan,
                         int32_t insno, int32_t reset_ctls);
int sensMidi(CSOUND *csound);

/* midirecv.c, resets MIDI controllers on a channel */
void midi_ctl_reset(CSOUND *csound, int16 chan);