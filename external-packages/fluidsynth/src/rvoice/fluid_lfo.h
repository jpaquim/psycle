/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#ifndef _FLUID_LFO_H
#define _FLUID_LFO_H

#include <utils/fluidsynth_priv.h>

typedef struct _fluid_lfo_t fluid_lfo_t;

struct _fluid_lfo_t {
	fluid_real_t val;          /* the current value of the LFO */
	unsigned int delay;       /* the delay of the lfo in samples */
	fluid_real_t increment;         /* the lfo frequency is converted to a per-buffer increment */
};

static inline void 
fluid_lfo_reset(fluid_lfo_t* lfo)
{
  lfo->val = 0.0f;
}

// These two cannot be inlined since they're used by event_dispatch
void fluid_lfo_set_incr(fluid_lfo_t* lfo, fluid_real_t increment);
void fluid_lfo_set_delay(fluid_lfo_t* lfo, unsigned int delay);

static inline fluid_real_t
fluid_lfo_get_val(fluid_lfo_t* lfo)
{
  return lfo->val;
}

static inline void 
fluid_lfo_calc(fluid_lfo_t* lfo, unsigned int cur_delay)
{
  if (cur_delay < lfo->delay) 
    return;
  
  lfo->val += lfo->increment;
  
  if (lfo->val > (fluid_real_t) 1.0)
  {
    lfo->increment = -lfo->increment;
    lfo->val = (fluid_real_t) 2.0 - lfo->val;
  }
  else if (lfo->val < (fluid_real_t) -1.0)
  {
    lfo->increment = -lfo->increment;
    lfo->val = (fluid_real_t) -2.0 - lfo->val;
  }

}

#endif

