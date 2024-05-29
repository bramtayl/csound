#pragma once

#include "csound.h"
#include "vbap.h"

int32_t     vbap_zak_init(CSOUND *, VBAP_ZAK *);
int32_t     vbap_zak(CSOUND *, VBAP_ZAK *);
int32_t     vbap_zak_moving_init(CSOUND *, VBAP_ZAK_MOVING *);
int32_t     vbap_zak_moving(CSOUND *, VBAP_ZAK_MOVING *);