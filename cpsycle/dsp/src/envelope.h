// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_ENVELOPE_H
#define psy_dsp_ENVELOPE_H

#include "dsptypes.h"
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	/// time at which to set the value. Unit can be different things depending on the context.
	psy_dsp_beat_t time;
	/// 0 .. 1.0f . (or -1.0 1.0 or whatever else) Use it as a multiplier.
	psy_dsp_amp_t value;
	psy_dsp_amp_t minvalue;
	psy_dsp_amp_t maxvalue;
	psy_dsp_beat_t mintime;
	psy_dsp_beat_t maxtime;	
} psy_dsp_EnvelopePoint;

void psy_dsp_envelopepoint_init(psy_dsp_EnvelopePoint*, 
	psy_dsp_seconds_t time,
	psy_dsp_amp_t value,
	psy_dsp_seconds_t mintime,
	psy_dsp_seconds_t maxtime,
	psy_dsp_amp_t minvalue,
	psy_dsp_amp_t maxvalue);

psy_dsp_EnvelopePoint* psy_dsp_envelopepoint_alloc(void);

psy_dsp_EnvelopePoint psy_dsp_envelopepoint_make_all(
	psy_dsp_seconds_t time,
	psy_dsp_amp_t value,
	psy_dsp_seconds_t mintime,
	psy_dsp_seconds_t maxtime,
	psy_dsp_amp_t minvalue,
	psy_dsp_amp_t maxvalue);

psy_dsp_EnvelopePoint psy_dsp_envelopepoint_make(
	psy_dsp_seconds_t time,
	psy_dsp_amp_t value);

psy_dsp_EnvelopePoint psy_dsp_envelopepoint_make_start(void);

// Mode defines what the first value of a PointValue means
// TICK = one tracker tick ( speed depends on the BPM )
// MILIS = a millisecond. (independant of BPM).
typedef enum {
	psy_dsp_ENVELOPETIME_TICK,
	psy_dsp_ENVELOPETIME_SECONDS
} psy_dsp_EnvelopeTimeMode;

/// The envelope is made of a list of pointvalues.
typedef psy_List* psy_dsp_EnvelopePoints;

// psy_dsp_Envelope
// defines an envelope and used by all internal machines (sampler, sampulse)
typedef struct psy_dsp_Envelope {
	/// Envelope is enabled or disabled
	bool enabled;
	/// if m_Carry and a new note enters, the envelope position is set to that
	/// of the previous note *on the same channel*
	bool carry;
	/// Array of Points of the envelope.
	psy_dsp_EnvelopePoints points;
	/// Loop Start Point
	uintptr_t loopstart;
	/// Loop End Point
	uintptr_t loopend;	
	/// Sustain Start Point
	uintptr_t sustainbegin;
	/// Sustain End Point
	uintptr_t sustainend;
	/// Envelope mode (meaning of the time value)
	psy_dsp_EnvelopeTimeMode timemode;
	/// tostring return string
	char* str;
} psy_dsp_Envelope;

void psy_dsp_envelope_init(psy_dsp_Envelope*);
void psy_dsp_envelope_init_adsr(psy_dsp_Envelope*);
void psy_dsp_envelope_init_dispose(psy_dsp_Envelope*);
void psy_dsp_envelope_init_copy(psy_dsp_Envelope* self,
	const psy_dsp_Envelope* source);
/// Appends a new point at the end of the list.
/// note: be sure that the pointtime is the highest of the points
void psy_dsp_envelope_append(psy_dsp_Envelope*,
	psy_dsp_EnvelopePoint);
/// Removes a point from the points Array.
void psy_dsp_envelope_delete(psy_dsp_Envelope*, 
	uintptr_t pointindex);
/// Clears the points Array
void psy_dsp_envelope_clear(psy_dsp_Envelope* self);

psy_dsp_EnvelopePoint psy_dsp_envelope_at(const psy_dsp_Envelope*,
	uintptr_t pointindex);
void psy_dsp_envelope_settimeandvalue(psy_dsp_Envelope*,
	uintptr_t pointindex, psy_dsp_seconds_t pointtime, psy_dsp_amp_t pointval);
void psy_dsp_envelope_settime(psy_dsp_Envelope*,
	uintptr_t pointindex, psy_dsp_seconds_t pointtime);

INLINE psy_dsp_seconds_t psy_dsp_envelope_time(
	const psy_dsp_Envelope* self, uintptr_t pointindex)
{
	psy_dsp_EnvelopePoint rv;

	rv = psy_dsp_envelope_at(self, pointindex);
	return rv.time;
}

void psy_dsp_envelope_setvalue(psy_dsp_Envelope*,
	uintptr_t pointindex, psy_dsp_amp_t pointval);

INLINE psy_dsp_amp_t psy_dsp_envelope_value(
	const psy_dsp_Envelope* self, uintptr_t pointindex)
{
	psy_dsp_EnvelopePoint rv;

	rv = psy_dsp_envelope_at(self, pointindex);
	return rv.value;
}

const char* psy_dsp_envelope_tostring(const psy_dsp_Envelope*);

INLINE bool psy_dsp_envelope_empty(const psy_dsp_Envelope* self)
{
	return self->points == NULL;
}

// Properties
/// Set or Get the point Index for Sustain and Loop.
INLINE uintptr_t psy_dsp_envelope_sustainbegin(
	const psy_dsp_Envelope* self)
{
	return self->sustainbegin;
}
/// value has to be an existing point!
INLINE void psy_dsp_envelope_setsustainbegin(
	psy_dsp_Envelope* self, const uintptr_t value)
{
	self->sustainbegin = value;
}

INLINE uintptr_t psy_dsp_envelope_sustainend(
	const psy_dsp_Envelope* self)
{
	return self->sustainend;
}
/// value has to be an existing point!
INLINE void psy_dsp_envelope_setsustainend(
	psy_dsp_Envelope* self, const uintptr_t value)
{
	self->sustainend = value;
}
INLINE uintptr_t psy_dsp_envelope_loopstart(
	const psy_dsp_Envelope* self)
{
	return self->loopstart;
}
/// value has to be an existing point!
INLINE void psy_dsp_envelope_setloopstart(
	psy_dsp_Envelope* self, const uintptr_t value)
{
	self->loopstart = value;
}

INLINE uintptr_t psy_dsp_envelope_loopend(
	const psy_dsp_Envelope* self)
{
	return self->loopend;
}
/// value has to be an existing point!
INLINE void psy_dsp_envelope_setloopend(
	psy_dsp_Envelope* self, const uintptr_t value)
{
	self->loopend = value;
}

INLINE uintptr_t psy_dsp_envelope_numofpoints(
	const psy_dsp_Envelope* self)
{
	return psy_list_size(self->points);	
}

//// If the envelope IsEnabled, it is used and triggered. Else, it is not.
INLINE bool psy_dsp_envelope_isenabled(
	const psy_dsp_Envelope* self)
{
	return self->enabled;
}

INLINE void psy_dsp_envelope_setenabled(
	psy_dsp_Envelope* self, const bool value)
{
	self->enabled = value;
}

// if IsCarry() and a new note enters, the envelope position is set to that
// of the previous note *on the same channel*
INLINE bool psy_dsp_envelope_iscarry(
	const psy_dsp_Envelope* self)
{
	return self->carry;
}

INLINE void psy_dsp_envelope_setcarry(
	psy_dsp_Envelope* self, const bool value)
{
	self->carry = value;
}

INLINE void psy_dsp_envelope_setmode(psy_dsp_Envelope* self,
	psy_dsp_EnvelopeTimeMode mode)
{
	self->timemode = mode;
}

INLINE psy_dsp_EnvelopeTimeMode psy_dsp_envelope_mode(
	const psy_dsp_Envelope* self) 
{
	return self->timemode;
}
// The extra parameters are used to convert the existing points from one unit
// to the other.
// void psy_dsp_envelope_mode(psy_dsp_Envelope* self,
//	const Mode::Type _mode, const int bpm = 125, const int tpb = 24, const int onemilli = 1);

// Only meaningful when used as adsr 
// INLINE void psy_dsp_envelope_AttackTime(int time, bool rezoom = true);
INLINE psy_dsp_seconds_t psy_dsp_envelope_attacktime(const
	psy_dsp_Envelope* self)
{
	assert(self);

	return psy_dsp_envelope_time(self, 1);	
}

INLINE void psy_dsp_envelope_setattacktime(
	psy_dsp_Envelope* self, psy_dsp_seconds_t time)
{
	assert(self);

	psy_dsp_envelope_settime(self, 1, time);
}

INLINE psy_dsp_seconds_t psy_dsp_envelope_decaytime(const
	psy_dsp_Envelope* self)
{
	assert(self);

	return psy_dsp_envelope_time(self, 2) -
		psy_dsp_envelope_time(self, 1);	
}

INLINE void psy_dsp_envelope_setdecaytime(
	psy_dsp_Envelope* self, psy_dsp_seconds_t time)
{
	assert(self);

	psy_dsp_envelope_settime(self, 2,
		psy_dsp_envelope_attacktime(self) + time);
}

INLINE void psy_dsp_envelope_setsustainvalue(
	psy_dsp_Envelope* self, psy_dsp_amp_t sustain)
{
	assert(self);

	psy_dsp_envelope_setvalue(self, 2, sustain);
}

INLINE psy_dsp_amp_t psy_dsp_envelope_sustainvalue(const
	psy_dsp_Envelope* self)
{
	assert(self);

	return psy_dsp_envelope_value(self, 2);
}

INLINE psy_dsp_seconds_t psy_dsp_envelope_releasetime(const
	psy_dsp_Envelope* self)
{
	assert(self);

	return psy_dsp_envelope_time(self, 3) -
		psy_dsp_envelope_time(self, 2);
}

INLINE void psy_dsp_envelope_setreleasetime(
	psy_dsp_Envelope* self, psy_dsp_seconds_t time)
{
	assert(self);

	psy_dsp_envelope_settime(self, 3,
		psy_dsp_envelope_attacktime(self) +
		psy_dsp_envelope_decaytime(self) +
		time);
}

// psy_dsp_EnvelopeController
// used by lua plugins and sampler ps1
// (sampulse has an own controller: xmenvelope_controller)
typedef struct psy_dsp_EnvelopeController {
	int rsvd;
	psy_dsp_Envelope settings;	
	psy_dsp_big_hz_t samplerate;
	psy_dsp_big_beat_t bpm;
	int tpb;
	psy_List* currstage;	
	psy_List* susbeginstage;
	psy_List* susendstage;
	psy_dsp_amp_t startpeak;
	psy_dsp_amp_t value;
	psy_dsp_amp_t step;	
	uintptr_t samplecount;
	uintptr_t nexttime;
	bool susdone;
	bool fastrelease;		
} psy_dsp_EnvelopeController;

void psy_dsp_envelopecontroller_init(psy_dsp_EnvelopeController*);
void psy_dsp_envelopecontroller_init_envelope(psy_dsp_EnvelopeController*,
	const psy_dsp_Envelope*);
void psy_dsp_envelopecontroller_init_adsr(psy_dsp_EnvelopeController*);
void psy_dsp_envelopecontroller_dispose(psy_dsp_EnvelopeController*);
void psy_dsp_envelopecontroller_reset(psy_dsp_EnvelopeController*);
void psy_dsp_envelopecontroller_set_settings(psy_dsp_EnvelopeController*,
	const psy_dsp_Envelope*);
void psy_dsp_envelopecontroller_settimeandvalue(psy_dsp_EnvelopeController*,
	uintptr_t pointindex, psy_dsp_seconds_t pointtime, psy_dsp_amp_t pointval);
void psy_dsp_envelopecontroller_setvalue(psy_dsp_EnvelopeController*,
	uintptr_t pointindex, psy_dsp_amp_t pointval);
psy_dsp_EnvelopePoint psy_dsp_envelopecontroller_at(const
	psy_dsp_EnvelopeController*, uintptr_t pointindex);
psy_List*  psy_dsp_envelopecontroller_begin(psy_dsp_EnvelopeController*);
void psy_dsp_envelopecontroller_setsamplerate(psy_dsp_EnvelopeController*,
	psy_dsp_big_hz_t samplerate);
void psy_dsp_envelopecontroller_updatespeed(psy_dsp_EnvelopeController*,
	uintptr_t tpb, double bpm);
psy_dsp_amp_t psy_dsp_envelopecontroller_tick(psy_dsp_EnvelopeController*);
psy_dsp_amp_t psy_dsp_envelopecontroller_tick_ps1(psy_dsp_EnvelopeController*);
void psy_dsp_envelopecontroller_start(psy_dsp_EnvelopeController*);
void psy_dsp_envelopecontroller_stop(psy_dsp_EnvelopeController*);
void psy_dsp_envelopecontroller_release(psy_dsp_EnvelopeController*);
void psy_dsp_envelopecontroller_fastrelease(psy_dsp_EnvelopeController*);

INLINE bool psy_dsp_envelopecontroller_releasing(psy_dsp_EnvelopeController* self)
{
	return (self->settings.points && self->currstage == self->settings.points->tail);
}

INLINE bool psy_dsp_envelopecontroller_playing(psy_dsp_EnvelopeController* self)
{	
	return self->currstage != NULL;	
}

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_ENVELOPE_H */
