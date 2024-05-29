#pragma once

#include "csound.h"

/**
 * Remove all configuration variables of Csound instance 'csound',
 * and free database. This function is called by csoundReset().
 * Return value is CSOUNDCFG_SUCCESS in case of success.
 */
int csoundDeleteAllConfigurationVariables(CSOUND *);