#include "cfgvar_internal.h"

#include <stddef.h>                  // for NULL

#include "csoundCore_internal.h"     // for CSOUND_
#include "csound_data_structures.h"  // for cs_cons_free, cs_hash_table_free
#include "memalloc.h"                // for mfree
#include "prototyp.h"                // for csoundDeleteAllConfigurationVari...

static int destroy_entire_db(CSOUND *csound, CS_HASH_TABLE *db) {
  CONS_CELL *head, *current;
  if (db == NULL)
    return CSOUNDCFG_SUCCESS;

  head = current = cs_hash_table_values(csound, db);

  while (current != NULL) {
    if (current->value != NULL) {
      mfree(csound, current->value);
    }
    current = current->next;
  }

  cs_cons_free(csound, head);
  cs_hash_table_free(csound, db);

  return CSOUNDCFG_SUCCESS;
}

/**
 * Remove all configuration variables of Csound instance 'csound'
 * and free database. This function is called by csoundReset().
 * Return value is CSOUNDCFG_SUCCESS in case of success.
 */

int csoundDeleteAllConfigurationVariables(CSOUND *csound) {
  int retval;
  retval = destroy_entire_db(csound, csound->cfgVariableDB);
  csound->cfgVariableDB = NULL;
  return retval;
}