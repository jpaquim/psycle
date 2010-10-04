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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */


#include "fluid_tuning.h"
#include <utils/fluidsynth_priv.h>
#include <utils/fluid_sys.h>


fluid_tuning_t* new_fluid_tuning(const char* name, int bank, int prog)
{
  fluid_tuning_t* tuning;
  int i;

  tuning = FLUID_NEW(fluid_tuning_t);
  if (tuning == NULL) {
    FLUID_LOG(FLUID_PANIC, "Out of memory");
    return NULL;
  }

  tuning->name = NULL;

  if (name != NULL) {
    tuning->name = FLUID_STRDUP(name);
  }

  tuning->bank = bank;
  tuning->prog = prog;

  for (i = 0; i < 128; i++) {
    tuning->pitch[i] = i * 100.0;
  }

  tuning->refcount = 1;         /* Start with a refcount of 1 */

  return tuning;
}

/* Duplicate a tuning */
fluid_tuning_t *
fluid_tuning_duplicate (fluid_tuning_t *tuning)
{
  fluid_tuning_t *new_tuning;
  int i;

  new_tuning = FLUID_NEW (fluid_tuning_t);

  if (!new_tuning) {
    FLUID_LOG (FLUID_PANIC, "Out of memory");
    return NULL;
  }

  if (tuning->name)
  {
    new_tuning->name = FLUID_STRDUP (tuning->name);

    if (!new_tuning->name)
    {
      FLUID_FREE (new_tuning);
      FLUID_LOG (FLUID_PANIC, "Out of memory");
      return NULL;
    }
  }
  else new_tuning->name = NULL;

  new_tuning->bank = tuning->bank;
  new_tuning->prog = tuning->prog;

  for (i = 0; i < 128; i++)
    new_tuning->pitch[i] = tuning->pitch[i];

  new_tuning->refcount = 1;     /* Start with a refcount of 1 */

  return new_tuning;
}

void
delete_fluid_tuning (fluid_tuning_t *tuning)
{
  if (tuning->name) FLUID_FREE (tuning->name);
  FLUID_FREE (tuning);
}

/* Add a reference to a tuning object */
void
fluid_tuning_ref (fluid_tuning_t *tuning)
{
  fluid_return_if_fail (tuning != NULL);

  fluid_atomic_int_inc (&tuning->refcount);
}

/* Unref a tuning object, when it reaches 0 it is deleted, returns TRUE if deleted */
int
fluid_tuning_unref (fluid_tuning_t *tuning, int count)
{
  fluid_return_val_if_fail (tuning != NULL, FALSE);

  /* Add and compare are separate, but that is OK, since refcount will only
   * reach 0 when there are no references and therefore no possibility of
   * another thread adding a reference in between */
  fluid_atomic_int_add (&tuning->refcount, -count);

  /* Delete when refcount reaches 0 */
  if (!fluid_atomic_int_get (&tuning->refcount))
  {
    delete_fluid_tuning (tuning);
    return TRUE;
  }
  else return FALSE;
}

void fluid_tuning_set_name(fluid_tuning_t* tuning, char* name)
{
  if (tuning->name != NULL) {
    FLUID_FREE(tuning->name);
    tuning->name = NULL;
  }
  if (name != NULL) {
    tuning->name = FLUID_STRDUP(name);
  }
}

char* fluid_tuning_get_name(fluid_tuning_t* tuning)
{
  return tuning->name;
}

void fluid_tuning_set_key(fluid_tuning_t* tuning, int key, double pitch)
{
  tuning->pitch[key] = pitch;
}

void fluid_tuning_set_octave(fluid_tuning_t* tuning, const double* pitch_deriv)
{
  int i;

  for (i = 0; i < 128; i++) {
    tuning->pitch[i] = i * 100.0 + pitch_deriv[i % 12];
  }
}

void fluid_tuning_set_all(fluid_tuning_t* tuning, const double* pitch)
{
  int i;

  for (i = 0; i < 128; i++) {
    tuning->pitch[i] = pitch[i];
  }
}

void fluid_tuning_set_pitch(fluid_tuning_t* tuning, int key, double pitch)
{
  if ((key >= 0) && (key < 128)) {
    tuning->pitch[key] = pitch;
  }
}
