// -*- mode:c++; indent-tabs-mode:t -*-
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

#ifndef _FLUIDSYNTH_AUDIO_H
#define _FLUIDSYNTH_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif



  /** Audio driver
   *
   *
   */


  /** The function should return non-zero if an error occured. */
typedef int (*fluid_audio_func_t)(void* data, int len, 
                                 int nin, float** in, 
                                 int nout, float** out);
/*
FLUIDSYNTH_API fluid_audio_driver_t* new_fluid_audio_driver(fluid_settings_t* settings, 
                                                         fluid_synth_t* synth);

FLUIDSYNTH_API fluid_audio_driver_t* new_fluid_audio_driver2(fluid_settings_t* settings, 
                                                          fluid_audio_func_t func,
                                                          void* data);

FLUIDSYNTH_API void delete_fluid_audio_driver(fluid_audio_driver_t* driver);
*/


#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_AUDIO_H */
