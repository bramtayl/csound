#pragma once

#include "csound.h"
#include "vbap.h"

int32_t vbap_control(CSOUND*, VBAP_DATA *p, MYFLT*, MYFLT*, MYFLT*);

int32_t     vbap_init(CSOUND *, VBAP *);
int32_t     vbap_init_a(CSOUND *, VBAPA *);
int32_t     vbap(CSOUND *, VBAP *);
int32_t     vbap_a(CSOUND *, VBAPA *);
int32_t     vbap_moving_init(CSOUND *, VBAP_MOVING *);
int32_t     vbap_moving(CSOUND *, VBAP_MOVING *);
int32_t     vbap_moving_init_a(CSOUND *, VBAPA_MOVING *);
int32_t     vbap_moving_a(CSOUND *, VBAPA_MOVING *);