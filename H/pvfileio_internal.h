#pragma once

#include "csound.h"
#include "pvfileio.h"

extern const GUID KSDATAFORMAT_SUBTYPE_PVOC;

/* pvoc file handling functions */

int32_t init_pvsys(CSOUND *);
int32_t pvsys_release(CSOUND *);
