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

#ifndef _FLUIDSYNTH_EVENT_H
#define _FLUIDSYNTH_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif


enum fluid_seq_event_type {
  FLUID_SEQ_NOTE = 0,
  FLUID_SEQ_NOTEON,
  FLUID_SEQ_NOTEOFF,
  FLUID_SEQ_ALLSOUNDSOFF,
  FLUID_SEQ_ALLNOTESOFF,
  FLUID_SEQ_BANKSELECT,
  FLUID_SEQ_PROGRAMCHANGE,
  FLUID_SEQ_PROGRAMSELECT,
  FLUID_SEQ_PITCHBEND,
  FLUID_SEQ_PITCHWHHELSENS,
  FLUID_SEQ_MODULATION,
  FLUID_SEQ_SUSTAIN,
  FLUID_SEQ_CONTROLCHANGE,
  FLUID_SEQ_PAN,
  FLUID_SEQ_VOLUME,
  FLUID_SEQ_REVERBSEND,
  FLUID_SEQ_CHORUSSEND,
  FLUID_SEQ_TIMER,
  FLUID_SEQ_ANYCONTROLCHANGE,	// used for remove_events only
  FLUID_SEQ_LASTEVENT  
};

/* Event alloc/free */
FLUIDSYNTH_API fluid_event_t* new_fluid_event(void);
FLUIDSYNTH_API void delete_fluid_event(fluid_event_t* evt);

/* Initializing events */
FLUIDSYNTH_API void fluid_event_set_source(fluid_event_t* evt, short src);
FLUIDSYNTH_API void fluid_event_set_dest(fluid_event_t* evt, short dest);

/* Timer events */
FLUIDSYNTH_API void fluid_event_timer(fluid_event_t* evt, void* data);

/* Note events */
FLUIDSYNTH_API void fluid_event_note(fluid_event_t* evt, int channel, 
				   short key, short vel, 
				   unsigned int duration);

FLUIDSYNTH_API void fluid_event_noteon(fluid_event_t* evt, int channel, short key, short vel);
FLUIDSYNTH_API void fluid_event_noteoff(fluid_event_t* evt, int channel, short key);
FLUIDSYNTH_API void fluid_event_all_sounds_off(fluid_event_t* evt, int channel);
FLUIDSYNTH_API void fluid_event_all_notes_off(fluid_event_t* evt, int channel);

/* Instrument selection */
FLUIDSYNTH_API void fluid_event_bank_select(fluid_event_t* evt, int channel, short bank_num);
FLUIDSYNTH_API void fluid_event_program_change(fluid_event_t* evt, int channel, short preset_num);
FLUIDSYNTH_API void fluid_event_program_select(fluid_event_t* evt, int channel, unsigned int sfont_id, short bank_num, short preset_num);

/* Real-time generic instrument controllers */
FLUIDSYNTH_API 
void fluid_event_control_change(fluid_event_t* evt, int channel, short control, short val);

/* Real-time instrument controllers shortcuts */
FLUIDSYNTH_API void fluid_event_pitch_bend(fluid_event_t* evt, int channel, int val);
FLUIDSYNTH_API void fluid_event_pitch_wheelsens(fluid_event_t* evt, int channel, short val);
FLUIDSYNTH_API void fluid_event_modulation(fluid_event_t* evt, int channel, short val);
FLUIDSYNTH_API void fluid_event_sustain(fluid_event_t* evt, int channel, short val);
FLUIDSYNTH_API void fluid_event_pan(fluid_event_t* evt, int channel, short val);
FLUIDSYNTH_API void fluid_event_volume(fluid_event_t* evt, int channel, short val);
FLUIDSYNTH_API void fluid_event_reverb_send(fluid_event_t* evt, int channel, short val);
FLUIDSYNTH_API void fluid_event_chorus_send(fluid_event_t* evt, int channel, short val);

/* Only for removing events */
FLUIDSYNTH_API void fluid_event_any_control_change(fluid_event_t* evt, int channel);

/* Accessing event data */
FLUIDSYNTH_API int fluid_event_get_type(fluid_event_t* evt);
FLUIDSYNTH_API short fluid_event_get_source(fluid_event_t* evt);
FLUIDSYNTH_API short fluid_event_get_dest(fluid_event_t* evt);
FLUIDSYNTH_API int fluid_event_get_channel(fluid_event_t* evt);
FLUIDSYNTH_API short fluid_event_get_key(fluid_event_t* evt);
FLUIDSYNTH_API short fluid_event_get_velocity(fluid_event_t* evt);
FLUIDSYNTH_API short fluid_event_get_control(fluid_event_t* evt);
FLUIDSYNTH_API short fluid_event_get_value(fluid_event_t* evt);
FLUIDSYNTH_API short fluid_event_get_program(fluid_event_t* evt);
FLUIDSYNTH_API void* fluid_event_get_data(fluid_event_t* evt);
FLUIDSYNTH_API unsigned int fluid_event_get_duration(fluid_event_t* evt);
FLUIDSYNTH_API short fluid_event_get_bank(fluid_event_t* evt);
FLUIDSYNTH_API int fluid_event_get_pitch(fluid_event_t* evt);
FLUIDSYNTH_API unsigned int fluid_event_get_sfont_id(fluid_event_t* evt);

#ifdef __cplusplus
}
#endif
#endif /* _FLUIDSYNTH_EVENT_H */
