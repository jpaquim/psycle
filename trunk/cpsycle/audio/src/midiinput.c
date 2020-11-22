// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "midiinput.h"
#include "constants.h"
#include "cmdsnotes.h"
#include "machine.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void psy_audio_midiinputstats_init(psy_audio_MidiInputStats* self)
{
	assert(self);

	self->flags = 0;	
	self->channelmapupdate = 0;
	self->channelmap = 0;
}

static void psy_audio_midiinput_setgenmap(psy_audio_MidiInput*, int channel,
	int generator);
static void psy_audio_midiinput_setinstmap(psy_audio_MidiInput*, int channel,
	int inst);

void psy_audio_midiinput_init(psy_audio_MidiInput* self)
{				
	psy_audio_midiconfig_init(&self->midiconfig);
	memset(self->channelinstmap, 0, sizeof(uintptr_t) * MAX_INSTRUMENTS);
	memset(self->channelgeneratormap, UINTPTR_MAX, sizeof(uintptr_t) *
		psy_audio_MAX_MIDI_CHANNELS);
	psy_audio_midiinputstats_init(&self->stats);
}

void psy_audio_midiinput_dispose(psy_audio_MidiInput* self)
{
	assert(self);

	psy_audio_midiconfig_dispose(&self->midiconfig);	
}

bool psy_audio_midiinput_workinput(psy_audio_MidiInput* self,
	psy_EventDriverMidiData mididata, psy_audio_Machines* machines,
	psy_audio_PatternEvent* rv)
{
	uintptr_t track = 0;
	int lsb;
	int msb;
	int status;
		
	assert(self);

	lsb = mididata.byte0 & 0x0F;
	msb = (mididata.byte0 & 0xF0) >> 4;

	rv->note = 0;
	rv->mach = 0;
	rv->inst = psy_audio_NOTECOMMANDS_INST_EMPTY;
	rv->vol = psy_audio_NOTECOMMANDS_VOL_EMPTY;
	rv->cmd = 0;
	rv->parameter = 0;

	status = mididata.byte0;

	if (mididata.byte0 != 0xFE) {
		self->stats.channelmap |= (1 << (lsb));
	}
	switch (self->midiconfig.gen_select_with) {
		case psy_audio_MIDICONFIG_MS_USE_SELECTED:
			psy_audio_midiinput_setgenmap(self, lsb,
				psy_audio_machines_slot(machines));
			break;
		case psy_audio_MIDICONFIG_MS_MIDI_CHAN:
			psy_audio_midiinput_setgenmap(self, lsb, lsb);
			break;
		default:
			break;
	}
	rv->mach = psy_audio_midiinput_genmap(self, lsb);	

	switch (msb) {
	case 0x8:
	case 0x9:
		// Note On/Off
		rv->note = (mididata.byte2 > 0)
			? mididata.byte1
			: 120;
		break;
	case 0xB:
		// switch on controller ID
		switch (mididata.byte1) {
			// BANK SELECT (controller 0)
		case 0: {
			// banks select -> map generator to channel
			if (self->midiconfig.gen_select_with == psy_audio_MIDICONFIG_MS_BANK) {
				// machine active?
				if (mididata.byte2 < MAX_MACHINES && psy_audio_machines_at(
						machines, mididata.byte2)) {
					// ok, map
					psy_audio_midiinput_setgenmap(self, lsb,
						mididata.byte2);
				} else {
					// machine not active, can't map!
					psy_audio_midiinput_setgenmap(self, lsb, -1);
				}
				return FALSE;
			} else if (self->midiconfig.inst_select_with ==
					psy_audio_MIDICONFIG_MS_BANK) {
				// banks select -> map instrument to channel
				psy_audio_midiinput_setinstmap(self, lsb, mididata.byte2);
				return FALSE;
			} else {
				rv->note = psy_audio_NOTECOMMANDS_MIDICC;
				rv->inst = (status & 0xF0) | (rv->inst & 0x0F);
				rv->cmd = mididata.byte1;
				rv->parameter = mididata.byte2;
				return FALSE;
			}
			break; }				
		default:
			break;
		}
	case 0xC:								
		// program change -> map generator/effect to channel
		if (self->midiconfig.gen_select_with ==
				psy_audio_MIDICONFIG_MS_PROGRAM) {
			// machine active?
			if (mididata.byte1 < MAX_MACHINES && psy_audio_machines_at(
				machines, mididata.byte1)) {
				// ok, map
				psy_audio_midiinput_setgenmap(self, lsb,
					mididata.byte1);
			} else {
				// machine not active, can't map!
				psy_audio_midiinput_setgenmap(self, lsb, -1);
			}
			return FALSE;
		} else if (self->midiconfig.inst_select_with ==
				psy_audio_MIDICONFIG_MS_PROGRAM) {
			// program select -> map instrument to channel
			psy_audio_midiinput_setinstmap(self, lsb, mididata.byte2);
			return FALSE;
		} else {
			rv->note = psy_audio_NOTECOMMANDS_MIDICC;
			rv->inst = (status & 0xF0) | (rv->inst & 0x0F);
			rv->cmd = mididata.byte1;
			rv->parameter = mididata.byte2;
			return FALSE;
		}
		break;
	case 0x0E:
		// pitch wheel
		// data 2 contains the info
		rv->note = psy_audio_NOTECOMMANDS_MIDICC;
		rv->inst = (status & 0xF0) | (rv->inst & 0x0F);
		rv->cmd = mididata.byte1;
		rv->parameter = mididata.byte2;
		break;
	default:
		return FALSE;
		break;
	}
		
	return TRUE;
}

void psy_audio_midiinput_setinstmap(psy_audio_MidiInput* self, int channel, int inst)
{
	assert(self);

	assert(channel < psy_audio_MAX_MIDI_CHANNELS);
	assert(inst < MAX_INSTRUMENTS);
	if (inst != self->channelinstmap[channel]) {
		self->channelinstmap[channel] = inst;
		++self->stats.channelmapupdate;
	}
}

int psy_audio_midiinput_instmap(psy_audio_MidiInput* self, int channel)
{
	assert(self);

	assert(channel < psy_audio_MAX_MIDI_CHANNELS);
	return self->channelinstmap[channel];
}

void psy_audio_midiinput_setgenmap(psy_audio_MidiInput* self, int channel,
	int generator)
{
	assert(self);

	assert(channel < psy_audio_MAX_MIDI_CHANNELS);
	// assert(generator < psy_audio_MAX_VIRTUALINSTS);
	if (generator != self->channelgeneratormap[channel]) {
		++self->stats.channelmapupdate;
		self->channelgeneratormap[channel] = generator;
	}
}

int psy_audio_midiinput_genmap(const psy_audio_MidiInput* self, int channel)
{
	assert(self);

	assert(channel < psy_audio_MAX_MIDI_CHANNELS);
	return self->channelgeneratormap[channel];
}

void psy_audio_midiinput_configure(psy_audio_MidiInput* self, psy_Property*
	configuration)
{
	assert(self);

	psy_audio_midiconfig_configure(&self->midiconfig, configuration);
}
