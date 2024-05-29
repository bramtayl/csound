#pragma once

#include "csoundCore_common.h"

TREE * csound_orc_optimize(CSOUND *csound, TREE *root);
TREE* constant_fold(CSOUND *csound, TREE* root);