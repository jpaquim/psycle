// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_ENVELOPE_H
#define psy_dsp_ENVELOPE_H

#include "dsptypes.h"
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Invalid point. Used to indicate that sustain/normal loop is disabled, or an out of range point.
/// BEWARE!!! UINTPTR_T, not minus one.
#define psy_dsp_ENVELOPEPOINT_INVALID UINTPTR_MAX

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

// Mode defines what the first value of a PointValue means
// TICK = one tracker tick ( speed depends on the BPM )
// MILIS = a millisecond. (independant of BPM).
typedef enum {
	psy_dsp_ENVELOPETIME_TICK,
	psy_dsp_ENVELOPETIME_SECONDS
} psy_dsp_EnvelopeTimeMode;

/// The envelope is made of a list of pointvalues.
typedef psy_List* psy_dsp_EnvelopePoints;

typedef struct psy_dsp_EnvelopeSettings {
	/// Envelope is enabled or disabled
	bool enabled;
	/// if m_Carry and a new note enters, the envelope position is set to that of the previous note *on the same channel*
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
} psy_dsp_EnvelopeSettings;

void psy_dsp_envelopesettings_init(psy_dsp_EnvelopeSettings*);
void psy_dsp_envelopesettings_init_adsr(psy_dsp_EnvelopeSettings*);
void psy_dsp_envelopesettings_dispose(psy_dsp_EnvelopeSettings*);
void psy_dsp_envelopesettings_copy(psy_dsp_EnvelopeSettings* self,
	const psy_dsp_EnvelopeSettings* source);
/// Appends a new point at the end of the list.
/// note: be sure that the pointtime is the highest of the points
void psy_dsp_envelopesettings_append(psy_dsp_EnvelopeSettings*,
	psy_dsp_EnvelopePoint);
/// Removes a point from the points Array.
void psy_dsp_envelopesettings_delete(psy_dsp_EnvelopeSettings*, 
	uintptr_t pointindex);
/// Clears the points Array
void psy_dsp_envelopesettings_clear(psy_dsp_EnvelopeSettings* self);

psy_dsp_EnvelopePoint psy_dsp_envelopesettings_at(const psy_dsp_EnvelopeSettings*,
	uintptr_t pointindex);
void psy_dsp_envelopesettings_settimeandvalue(psy_dsp_EnvelopeSettings*,
	uintptr_t pointindex, psy_dsp_seconds_t pointtime, psy_dsp_amp_t pointval);
void psy_dsp_envelopesettings_settime(psy_dsp_EnvelopeSettings*,
	uintptr_t pointindex, psy_dsp_seconds_t pointtime);

INLINE psy_dsp_seconds_t psy_dsp_envelopesettings_time(
	const psy_dsp_EnvelopeSettings* self, uintptr_t pointindex)
{
	psy_dsp_EnvelopePoint rv;

	rv = psy_dsp_envelopesettings_at(self, pointindex);
	return rv.time;
}

void psy_dsp_envelopesettings_setvalue(psy_dsp_EnvelopeSettings*,
	uintptr_t pointindex, psy_dsp_amp_t pointval);

INLINE psy_dsp_amp_t psy_dsp_envelopesettings_value(
	const psy_dsp_EnvelopeSettings* self, uintptr_t pointindex)
{
	psy_dsp_EnvelopePoint rv;

	rv = psy_dsp_envelopesettings_at(self, pointindex);
	return rv.value;
}

const char* psy_dsp_envelopesettings_tostring(const psy_dsp_EnvelopeSettings*);

INLINE bool psy_dsp_envelopesettings_empty(const psy_dsp_EnvelopeSettings* self)
{
	return self->points == NULL;
}

// Properties
/// Set or Get the point Index for Sustain and Loop.
INLINE uintptr_t psy_dsp_envelopesettings_sustainbegin(
	const psy_dsp_EnvelopeSettings* self)
{
	return self->sustainbegin;
}
/// value has to be an existing point!
INLINE void psy_dsp_envelopesettings_setsustainbegin(
	psy_dsp_EnvelopeSettings* self, const uintptr_t value)
{
	self->sustainbegin = value;
}

INLINE uintptr_t psy_dsp_envelopesettings_sustainend(
	const psy_dsp_EnvelopeSettings* self)
{
	return self->sustainend;
}
/// value has to be an existing point!
INLINE void psy_dsp_envelopesettings_setsustainend(
	psy_dsp_EnvelopeSettings* self, const uintptr_t value)
{
	self->sustainend = value;
}
INLINE uintptr_t psy_dsp_envelopesettings_loopstart(
	const psy_dsp_EnvelopeSettings* self)
{
	return self->loopstart;
}
/// value has to be an existing point!
INLINE void psy_dsp_envelopesettings_setloopstart(
	psy_dsp_EnvelopeSettings* self, const uintptr_t value)
{
	self->loopstart = value;
}

INLINE uintptr_t psy_dsp_envelopesettings_loopend(
	const psy_dsp_EnvelopeSettings* self)
{
	return self->loopend;
}
/// value has to be an existing point!
INLINE void psy_dsp_envelopesettings_setloopend(
	psy_dsp_EnvelopeSettings* self, const uintptr_t value)
{
	self->loopend = value;
}

INLINE uintptr_t psy_dsp_envelopesettings_numofpoints(
	const psy_dsp_EnvelopeSettings* self)
{
	return psy_list_size(self->points);	
}

//// If the envelope IsEnabled, it is used and triggered. Else, it is not.
INLINE bool psy_dsp_envelopesettings_isenabled(
	const psy_dsp_EnvelopeSettings* self)
{
	return self->enabled;
}

INLINE void psy_dsp_envelopesettings_setenabled(
	psy_dsp_EnvelopeSettings* self, const bool value)
{
	self->enabled = value;
}

/// if IsCarry() and a new note enters, the envelope position is set to that of the previous note *on the same channel*
INLINE bool psy_dsp_envelopesettings_iscarry(
	const psy_dsp_EnvelopeSettings* self)
{
	return self->carry;
}

INLINE void psy_dsp_envelopesettings_setcarry(
	psy_dsp_EnvelopeSettings* self, const bool value)
{
	self->carry = value;
}

INLINE psy_dsp_EnvelopeTimeMode psy_dsp_envelopesettings_mode(
	const psy_dsp_EnvelopeSettings* self) 
{
	return self->timemode;
}
//The extra parameters are used to convert the existing points from one unit to the other.
//void psy_dsp_envelopesettings_mode(psy_dsp_EnvelopeSettings* self,
//	const Mode::Type _mode, const int bpm = 125, const int tpb = 24, const int onemilli = 1);


// Only meaningful when used as adsr 
// INLINE void psy_dsp_envelopesettings_AttackTime(int time, bool rezoom = true);
INLINE psy_dsp_seconds_t psy_dsp_envelopesettings_attacktime(const
	psy_dsp_EnvelopeSettings* self)
{
	assert(self);

	return psy_dsp_envelopesettings_time(self, 1);	
}

INLINE void psy_dsp_envelopesettings_setattacktime(
	psy_dsp_EnvelopeSettings* self, psy_dsp_seconds_t time)
{
	assert(self);

	psy_dsp_envelopesettings_settime(self, 1, time);
}

INLINE psy_dsp_seconds_t psy_dsp_envelopesettings_decaytime(const
	psy_dsp_EnvelopeSettings* self)
{
	assert(self);

	return psy_dsp_envelopesettings_time(self, 2) -
		psy_dsp_envelopesettings_time(self, 1);	
}

INLINE void psy_dsp_envelopesettings_setdecaytime(
	psy_dsp_EnvelopeSettings* self, psy_dsp_seconds_t time)
{
	assert(self);

	psy_dsp_envelopesettings_settime(self, 2,
		psy_dsp_envelopesettings_attacktime(self) + time);
}

INLINE void psy_dsp_envelopesettings_setsustainvalue(
	psy_dsp_EnvelopeSettings* self, psy_dsp_amp_t sustain)
{
	assert(self);

	psy_dsp_envelopesettings_setvalue(self, 2, sustain);
}

INLINE psy_dsp_amp_t psy_dsp_envelopesettings_sustainvalue(const
	psy_dsp_EnvelopeSettings* self)
{
	assert(self);

	return psy_dsp_envelopesettings_value(self, 2);
}

INLINE psy_dsp_seconds_t psy_dsp_envelopesettings_releasetime(const
	psy_dsp_EnvelopeSettings* self)
{
	assert(self);

	return psy_dsp_envelopesettings_time(self, 3) -
		psy_dsp_envelopesettings_time(self, 2);
}

INLINE void psy_dsp_envelopesettings_setreleasetime(
	psy_dsp_EnvelopeSettings* self, psy_dsp_seconds_t time)
{
	assert(self);

	psy_dsp_envelopesettings_settime(self, 3,
		psy_dsp_envelopesettings_attacktime(self) +
		psy_dsp_envelopesettings_decaytime(self) +
		time);
}

typedef struct psy_dsp_Envelope {
	int rsvd;
	psy_dsp_EnvelopeSettings settings;	
	uintptr_t samplerate;
	float bpm;
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
} psy_dsp_Envelope;

void psy_dsp_envelope_init(psy_dsp_Envelope*);
void psy_dsp_envelope_init_adsr(psy_dsp_Envelope*);
void psy_dsp_envelope_dispose(psy_dsp_Envelope*);
void psy_dsp_envelope_reset(psy_dsp_Envelope*);
void psy_dsp_envelope_set_settings(psy_dsp_Envelope*,
	const psy_dsp_EnvelopeSettings*);
void psy_dsp_envelope_settimeandvalue(psy_dsp_Envelope*, uintptr_t pointindex,
	psy_dsp_seconds_t pointtime, psy_dsp_amp_t pointval);
void psy_dsp_envelope_setvalue(psy_dsp_Envelope*, uintptr_t pointindex,
	psy_dsp_amp_t pointval);
psy_dsp_EnvelopePoint psy_dsp_envelope_at(const psy_dsp_Envelope*,
	uintptr_t pointindex);
psy_List*  psy_dsp_envelope_begin(psy_dsp_Envelope*);
void psy_dsp_envelope_setsamplerate(psy_dsp_Envelope*, uintptr_t samplerate);
void psy_dsp_envelope_updatespeed(psy_dsp_Envelope*, int tpb, int bpm);
psy_dsp_amp_t psy_dsp_envelope_tick(psy_dsp_Envelope*);
psy_dsp_amp_t psy_dsp_envelope_tick_ps1(psy_dsp_Envelope*);
void psy_dsp_envelope_start(psy_dsp_Envelope*);
void psy_dsp_envelope_stop(psy_dsp_Envelope*);
void psy_dsp_envelope_release(psy_dsp_Envelope*);
void psy_dsp_envelope_fastrelease(psy_dsp_Envelope*);

INLINE bool psy_dsp_envelope_releasing(psy_dsp_Envelope* self)
{
	return (self->settings.points && self->currstage == self->settings.points->tail);
}

INLINE bool psy_dsp_envelope_playing(psy_dsp_Envelope* self)
{	
	return self->currstage != NULL;	
}

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_ENVELOPE_H */
