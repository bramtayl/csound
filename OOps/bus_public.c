#include "bus_public.h"

#include "bus.h"
#include "csoundCore_internal.h"

PUBLIC int csoundGetChannelPtr(CSOUND *csound, MYFLT **p, const char *name,
                               int type) {
  CHNENTRY *pp;

  *p = (MYFLT *)NULL;
  if (UNLIKELY(name == NULL))
    return CSOUND_ERROR;
  pp = find_channel(csound, name);
  if (!pp) {
    if (create_new_channel(csound, name, type) == CSOUND_SUCCESS) {
      pp = find_channel(csound, name);
    }
  }
  if (pp != NULL) {
    if ((pp->type ^ type) & CSOUND_CHANNEL_TYPE_MASK)
      return pp->type;
    pp->type |= (type & (CSOUND_INPUT_CHANNEL | CSOUND_OUTPUT_CHANNEL));
    *p = pp->data;
    return CSOUND_SUCCESS;
  }
  return CSOUND_ERROR;
}

PUBLIC int csoundListChannels(CSOUND *csound, controlChannelInfo_t **lst) {
  CHNENTRY *pp;
  size_t n;
  CONS_CELL *channels;

  *lst = (controlChannelInfo_t *)NULL;
  if (csound->chn_db == NULL)
    return 0;

  channels = cs_hash_table_values(csound, csound->chn_db);
  n = cs_cons_length(channels);

  if (!n)
    return 0;

  /* create list, initially in unsorted order */
  //  mmalloc and the caller has to free it.
  // if not, it will be freed on reset
  *lst =
      (controlChannelInfo_t *)mmalloc(csound, n * sizeof(controlChannelInfo_t));
  if (UNLIKELY(*lst == NULL))
    return CSOUND_MEMORY;

  n = 0;
  while (channels != NULL) {
    pp = channels->value;
    (*lst)[n].name = pp->name;
    (*lst)[n].type = pp->type;
    (*lst)[n].hints = pp->hints;
    channels = channels->next;
    n++;
  }

  /* sort list */
  qsort((void *)(*lst), n, sizeof(controlChannelInfo_t), bus_cmp_func);
  /* return the number of channels */
  return (int32_t)n;
}

PUBLIC void csoundDeleteChannelList(CSOUND *csound, controlChannelInfo_t *lst) {
  //(void) csound;
  if (lst != NULL)
    mfree(csound, lst);
}

PUBLIC int csoundSetControlChannelHints(CSOUND *csound, const char *name,
                                        controlChannelHints_t hints) {
  CHNENTRY *pp;

  if (UNLIKELY(name == NULL))
    return CSOUND_ERROR;
  pp = find_channel(csound, name);
  if (UNLIKELY(pp == NULL))
    return CSOUND_ERROR;
  if (UNLIKELY((pp->type & CSOUND_CHANNEL_TYPE_MASK) != CSOUND_CONTROL_CHANNEL))
    return CSOUND_ERROR;
  if (hints.behav == CSOUND_CONTROL_CHANNEL_NO_HINTS) {
    pp->hints.behav = CSOUND_CONTROL_CHANNEL_NO_HINTS;
    return 0;
  }
  if (hints.behav == CSOUND_CONTROL_CHANNEL_INT) {
    hints.dflt = (MYFLT)((int32_t)MYFLT2LRND(hints.dflt));
    hints.min = (MYFLT)((int32_t)MYFLT2LRND(hints.min));
    hints.max = (MYFLT)((int32_t)MYFLT2LRND(hints.max));
  }
  if (UNLIKELY(hints.min > hints.max || hints.dflt < hints.min ||
               hints.dflt > hints.max ||
               (hints.behav == CSOUND_CONTROL_CHANNEL_EXP &&
                ((hints.min * hints.max) <= FL(0.0))))) {
    return CSOUND_ERROR;
  }

  pp->hints = hints;
  if (hints.attributes) {
    pp->hints.attributes =
        (char *)mmalloc(csound, (strlen(hints.attributes) + 1) * sizeof(char));
    strcpy(pp->hints.attributes, hints.attributes);
  }
  return CSOUND_SUCCESS;
}

/**
 * Returns special parameters (assuming there are any) of a control channel,
 * previously set with csoundSetControlChannelHints().
 * If the channel exists, is a control channel, and has the special parameters
 * assigned, then the default, minimum, and maximum value is stored in *dflt,
 * *min, and *max, respectively, and a positive value that is one of
 * CSOUND_CONTROL_CHANNEL_INT, CSOUND_CONTROL_CHANNEL_LIN, and
 * CSOUND_CONTROL_CHANNEL_EXP is returned.
 * In any other case, *dflt, *min, and *max are not changed, and the return
 * value is zero if the channel exists, is a control channel, but has no
 * special parameters set; otherwise, a negative error code is returned.
 */

PUBLIC int csoundGetControlChannelHints(CSOUND *csound, const char *name,
                                        controlChannelHints_t *hints) {
  CHNENTRY *pp;

  if (UNLIKELY(name == NULL))
    return CSOUND_ERROR;
  pp = find_channel(csound, name);
  if (pp == NULL)
    return CSOUND_ERROR;
  if ((pp->type & CSOUND_CHANNEL_TYPE_MASK) != CSOUND_CONTROL_CHANNEL)
    return CSOUND_ERROR;
  if (pp->hints.behav == 0)
    return CSOUND_ERROR;
  *hints = pp->hints;
  if (pp->hints.attributes) {
    hints->attributes =
        (char *)mmalloc(csound, strlen(pp->hints.attributes) + 1);
    strcpy(hints->attributes, pp->hints.attributes);
  }
  return 0;
}

PUBLIC int *csoundGetChannelLock(CSOUND *csound, const char *name) {
  CHNENTRY *pp;

  if (UNLIKELY(name == NULL))
    return NULL;
  pp = find_channel(csound, name);
  if (pp) {
    return (int32_t *)&pp->lock;
  } else
    return NULL;
}

PUBLIC int csoundGetChannelDatasize(CSOUND *csound, const char *name) {

  CHNENTRY *pp;
  pp = find_channel(csound, name);
  if (pp == NULL)
    return 0;
  else {
    /* the reason for this is that if chnexport is
       used with strings, the datasize might become
       invalid */
    if ((pp->type & CSOUND_STRING_CHANNEL) == CSOUND_STRING_CHANNEL)
      return ((STRINGDAT *)pp->data)->size;
    return pp->datasize;
  }
}
