// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "vstevents.h"
#if defined(__GNUC__)
#define _inline static inline
#endif
#include "aeffectx.h"

#include "../../detail/portable.h"
#include "../../detail/trace.h"

static struct VstMidiEvent* allocmidi(uint8_t data0, uint8_t data1, uint8_t data2)
{
	struct VstMidiEvent* rv;

	rv = malloc(sizeof(struct VstMidiEvent));
	if (rv) {
		memset(rv, 0, sizeof(struct VstMidiEvent));
		rv->type = kVstMidiType;
		rv->byteSize = sizeof(struct VstMidiEvent);
		rv->flags = kVstMidiEventIsRealtime;
		rv->midiData[0] = (char)data0;
		rv->midiData[1] = (char)data1;
		rv->midiData[2] = (char)data2;
	}
	return rv;
}


// psy_audio_VstEvents
void psy_audio_vstevents_init(psy_audio_VstEvents* self, uintptr_t capacity)
{
	self->eventcap = capacity;
	self->counter = 0;
	if (capacity > 0) {
		uintptr_t size;
		uintptr_t i;
				
		size = sizeof(struct VstEvents) + sizeof(VstEvent*) * self->eventcap;
		self->events = (struct VstEvents*)malloc(size);
		if (self->events) {
			self->events->reserved = 0;
			self->events->numEvents = 0;
			for (i = 0; i < self->eventcap; ++i) {
				self->events->events[i] = NULL;
			}
		}
	} else {
		self->events = NULL;
	}
}

void psy_audio_vstevents_dispose(psy_audio_VstEvents* self)
{
	psy_audio_vstevents_clear(self);
	free(self->events);
	self->events = 0;
	self->eventcap = 0;	
}

void psy_audio_vstevents_clear(psy_audio_VstEvents* self)
{
	if (self->events) {
		VstInt32 i;

		for (i = 0; i < self->events->numEvents; ++i) {
			free(self->events->events[i]);
			self->events->events[i] = NULL;
		}
		self->events->numEvents = 0;
		self->counter = 0;
	}
}

void psy_audio_vstevents_append(psy_audio_VstEvents* self, VstEvent* ev)
{
	if (self->counter < self->eventcap) {
		self->events->events[self->counter++] = ev;
		self->events->numEvents = (VstInt32)self->counter;
	}
}

void psy_audio_vstevents_append_midi_control(psy_audio_VstEvents* self,
	uint8_t channel, uint8_t cc, uint8_t data)
{
	psy_audio_vstevents_append_midi(self, 0xB0 | channel, cc, 0);	
}

void psy_audio_vstevents_append_noteon(psy_audio_VstEvents* self,
	uint8_t channel, uint8_t note)
{
	psy_audio_vstevents_append_midi(self, 0x90 | channel, note, 127);	
}

void psy_audio_vstevents_append_noteoff(psy_audio_VstEvents* self,
	uint8_t channel, uint8_t note)
{
	psy_audio_vstevents_append_midi(self, 0x80 | channel, note, 0);
}

void psy_audio_vstevents_append_midi(psy_audio_VstEvents* self,
	uint8_t byte0, uint8_t byte1, uint8_t byte2)
{
	if (self->counter < self->eventcap) {
		psy_audio_vstevents_append(self, (VstEvent*)
			allocmidi(byte0, byte1, byte2));
	}
}

