#pragma once

#include "csound.h"
#include "vbap.h"

int32_t     vbap1_init(CSOUND *, VBAP1 *);
int32_t     vbap1(CSOUND *, VBAP1 *);
int32_t     vbap1_init_a(CSOUND *, VBAPA1 *);
int32_t     vbap1a(CSOUND *, VBAPA1 *);
int32_t     vbap1_moving_init(CSOUND *, VBAP1_MOVING *);
int32_t     vbap1_moving(CSOUND *, VBAP1_MOVING *);
int32_t     vbap1_moving_init_a(CSOUND *, VBAPA1_MOVING *);
int32_t
vbap1_moving_a(CSOUND *, VBAPA1_MOVING *);