/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#include "midiinput.h"
#include "constants.h"
#include "cmdsnotes.h"
#include "machine.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void psy_audio_setcontrollermap(psy_audio_MidiInput*, int channel,
	int controller, int parameter);
static int psy_audio_getcontrollermap(psy_audio_MidiInput* self, int channel,
	int controller);

void psy_audio_midiinputstats_init(psy_audio_MidiInputStats* self)
{
	assert(self);

	self->flags = 0;	
	self->channelmapupdate = 0;
	self->channelmap = 0;	
}

void psy_audio_midiinput_init(psy_audio_MidiInput* self, psy_audio_Song* song)
{	
	uintptr_t i;
				
	psy_audio_midiconfig_init(&self->midiconfig);
	memset(self->channelsetting, -1, sizeof(int) * psy_audio_MAX_MIDI_CHANNELS);
	memset(self->channelinstmap, 0, sizeof(uintptr_t) * MAX_INSTRUMENTS);
	for (i = 0; i < psy_audio_MAX_MIDI_CHANNELS; ++i) {
		self->channelgeneratormap[i] = psy_INDEX_INVALID;
	}	
	memset(self->channelnoteoff, TRUE, sizeof(bool) *
		psy_audio_MAX_MIDI_CHANNELS);
	memset(self->channelcontroller, -1, sizeof(int) * psy_audio_MAX_MIDI_CHANNELS *
		psy_audio_MAX_CONTROLLERS);
	psy_audio_midiinputstats_init(&self->stats);
	self->song = song;
}

void psy_audio_midiinput_dispose(psy_audio_MidiInput* self)
{
	assert(self);

	psy_audio_midiconfig_dispose(&self->midiconfig);	
}

void psy_audio_midiinput_setsong(psy_audio_MidiInput* self, psy_audio_Song* song)
{
	assert(self);

	self->song = song;
}

bool psy_audio_midiinput_work_input(psy_audio_MidiInput* self,
	psy_EventDriverMidiData mididata, psy_audio_Machines* machines,
	psy_audio_PatternEvent* rv)
{
	uintptr_t track = 0;
	int status_ln;
	int status_hn;
	int status;
	int channel;

	int note;
	int program;
	int data1;
	int velocity;
	int data2;

	int busmachine;
	int inst;
	int cmd;
	int parameter;
	int orignote;

	assert(self);

	psy_audio_patternevent_clear(rv);

	status = mididata.byte0;
	status_ln = mididata.byte0 & 0x0F;
	status_hn = (mididata.byte0 & 0xF0) >> 4;
	channel = status_ln;
	
	/* assign uses */
	note = mididata.byte1;
	program = mididata.byte1;
	data1 = mididata.byte1;
	velocity = mididata.byte2;
	data2 = mididata.byte2;

		
	if (mididata.byte0 != 0xFE) {
		self->stats.channelmap |= (1 << (channel));
	}
	/* map channel -> generator */
	busmachine = (int)psy_audio_midiinput_genmap(self, channel);
	inst = (int)psy_audio_midiinput_instmap(self, channel);
	cmd = 0;
	parameter = 0;
	orignote = psy_audio_NOTECOMMANDS_EMPTY;

	switch (status_hn) {
	case 0x08:	/* (also) note off */
		velocity = 0;
		/* fallthrough */
	case 0x9:
		// limit to playable range (above this is special codes)
		if (note > psy_audio_NOTECOMMANDS_B9) note = psy_audio_NOTECOMMANDS_B9;
		// note on?
		if (velocity)
		{
			cmd = 0x0C;
			parameter = velocity * 2;
		}
		// note off
		else if (self->channelnoteoff[channel])
		{
			orignote = note;
			note = psy_audio_NOTECOMMANDS_RELEASE;
		} else
		{
			return FALSE;
		}
		break;
		// controller
	case 0xB:
	{
		// switch on controller ID
		switch (data1)
		{
			// BANK SELECT (controller 0)
		case 0:
		{
			// banks select -> map generator to channel
			if (self->midiconfig.gen_select_with == psy_audio_MIDICONFIG_MS_BANK) {
				// machine active?
				if (mididata.byte2 < MAX_MACHINES && psy_audio_machines_at(
					machines, data2)) {
					// ok, map
					psy_audio_midiinput_setgenmap(self, channel,
						data2);
				} else {
					// machine not active, can't map!
					psy_audio_midiinput_setgenmap(self, channel, psy_INDEX_INVALID);
				}
				return FALSE;
			} else if (self->midiconfig.inst_select_with ==
				psy_audio_MIDICONFIG_MS_BANK) {
				// banks select -> map instrument to channel
				psy_audio_midiinput_setinstmap(self, channel, data2);
				return FALSE;
			} else {
				rv->note = psy_audio_NOTECOMMANDS_MIDICC;
				rv->inst = (status & 0xF0) | (inst & 0x0F);
				rv->cmd = data1;
				rv->parameter = data2;
			}
			break; }
		// NOTE OFF ENABLE (controller 125)
		case 0x7D:
		{
			// enable/disable
			if (data2)
			{
				self->channelnoteoff[channel] = TRUE;
			} else
			{
				self->channelnoteoff[channel] = FALSE;
			}
			++self->stats.channelmapupdate;
			return FALSE;
		}
		break;
		// SET CONTROLLER (stage 1 - controller 126)
		case 0x7E:
		{
			self->channelsetting[channel] = data2;
			return FALSE;
		}
		break;
		// SET CONTROLLER (stage 2 - controller 127)
		case 0x7F:
		{
			// controller number set? (stage1)
			if (self->channelsetting[channel] >= 0)
			{
				// we can set map
				psy_audio_setcontrollermap(self, channel,
					self->channelsetting[channel], data2);

				// clear down
				self->channelsetting[channel] = -1;
			}
			return FALSE;
		}
		break;
		// * ANY OTHER CONTROLLER COMES HERE *
		default:
		{
			// generic controller -> tweak
			int gParameter = psy_audio_getcontrollermap(self, channel, data1);

			// set?
			if (gParameter >= 0)
			{
				psy_audio_Machine* machine;

				note = psy_audio_NOTECOMMANDS_TWEAK;
				inst = gParameter;

				machine = psy_audio_machines_at(&self->song->machines,
					psy_audio_machines_selected(&self->song->machines));
				if (machine && inst < (int)psy_audio_machine_numtweakparameters(machine))
				{					
					psy_audio_MachineParam* param;

					param = psy_audio_machine_tweakparameter(machine, inst);
					if (param) {
						intptr_t minval, maxval;
						intptr_t value;

						psy_audio_machine_parameter_range(machine, param, &minval, &maxval);
						value = minval + (intptr_t)(data2 * (maxval - minval) / 127.f + 0.5);
						cmd = (int)value / 256;
						parameter = value % 256;
					}									
				} else {
					parameter = data2;
				}
			} else if (self->midiconfig.raw)
			{
				note = psy_audio_NOTECOMMANDS_MIDICC;
				inst = (status & 0xF0) | (inst & 0x0F);
				cmd = data1;
				parameter = data2;
			} else
			{
				// search if there's a remap
				psy_List* i;

				for (i = self->midiconfig.groups; i != NULL; psy_list_next(&i))
				{
					psy_audio_MidiConfigGroup* group;

					group = (psy_audio_MidiConfigGroup*)psy_list_entry(i);
					if (group->record && group->message == data1)
					{
						int value;
						
						value = (int)(group->from + (group->to - group->from) * data2 / 127);
						switch (group->type)
						{
						case psy_audio_MIDICONFIG_T_COMMAND:
							note = psy_audio_NOTECOMMANDS_EMPTY;
							inst = psy_audio_NOTECOMMANDS_INST_EMPTY;
							cmd = group->command;
							parameter = value;
							break;
						case psy_audio_MIDICONFIG_T_TWEAK:
							note = psy_audio_NOTECOMMANDS_TWEAK;
							inst = group->command;
							cmd = (value >> 8) & 255;
							parameter = value & 255;
							break;
						case psy_audio_MIDICONFIG_T_TWEAKSLIDE:
							note = psy_audio_NOTECOMMANDS_TWEAKSLIDE;
							inst = group->command;
							cmd = (value >> 8) & 255;
							parameter = value & 255;
							break;
						case psy_audio_MIDICONFIG_T_MCM:
							note = psy_audio_NOTECOMMANDS_MIDICC;
							inst = group->command | (inst & 0x0F);
							cmd = data1;
							parameter = data2;
							break;
						}
						break;
					}
				}

				// controller not setup, we can't do anything
				if (i == NULL)
					return FALSE;
			}
		}
		break;
		}
	}
	break;
	case 0xC:
	{
		// program change -> map generator/effect to channel
		if (self->midiconfig.gen_select_with == psy_audio_MIDICONFIG_MS_PROGRAM)
		{
			// machine active?
			if (program < MAX_MACHINES && psy_audio_machines_at(&self->song->machines,
				program))
			{
				// ok, map
				psy_audio_midiinput_setgenmap(self, channel, program);
			} else
			{
				// machine not active, can't map!
				psy_audio_midiinput_setgenmap(self, channel, psy_INDEX_INVALID);
			}
			return FALSE;
		} else if (self->midiconfig.gen_select_with == psy_audio_MIDICONFIG_MS_PROGRAM)
		{
			psy_audio_midiinput_setinstmap(self, channel, program);
			return FALSE;
		} else {
			note = psy_audio_NOTECOMMANDS_MIDICC;
			inst = 0xC0 | (inst & 0x0F);
			cmd = program;
		}
	}
	break;
	case 0x0E:
		// pitch wheel
		// data 2 contains the info
		note = psy_audio_NOTECOMMANDS_MIDICC;
		inst = (status & 0xF0) | (inst & 0x0F);
		cmd = data1;
		parameter = data2;
		break;
		// extended codes
	case 0x0F:
	{
		switch (status_ln) {
			// MIDI SEEK: Set Song Position Pointer
			case 0x02:
				note = psy_audio_NOTECOMMANDS_MIDI_SPP;
				// data1 and data2 needs to be combined into a 14-bit value
				// The 14-bit value is the MIDI Beat (1 MIDI Beat = 1/16 beat)
				cmd = data1;
				parameter = data2;
				break;
			case 0x0A: {
				note = psy_audio_NOTECOMMANDS_MIDI_CLK_START;
				self->stats.flags |= FSTAT_FASTART;

				// force sync
				// InternalReSync(dwTime);
				// return FALSE;		
				break; }
				 // MIDI SYNC: Timing Clock
			case 0x08: {
				note = psy_audio_NOTECOMMANDS_MIDI_CLK;
				self->stats.flags |= FSTAT_F8CLOCK;

				// resync?
				//if (m_reSync)
				///{
					//m_stats.flags |= FSTAT_RESYNC;

					//m_reSync = false;

					// force sync
					//InternalReSync(dwTime);
				//} else
				//{
					// use clocks to keep us in sync as best as we can
					//InternalClock(dwTime);
				//}
				// return FALSE;
				break; }
			case 0x0B: {
				note = psy_audio_NOTECOMMANDS_MIDI_CLK_CONT;
				self->stats.flags |= FSTAT_FASTART;
			break; }
			// MIDI SYNC: Stop
			case 0x0C: {
				note = psy_audio_NOTECOMMANDS_MIDI_CLK_STOP;
				self->stats.flags |= FSTAT_FCSTOP;

				// stop the song play (in effect, stops all sound)
				// handled by psy_audio_sequencer_clock_stop
				break; }
					 // do nothing (apart from exit) if not recognised
			default:		
				return FALSE;
				break;
		}
		break; }
	default:
		return FALSE;
		break;
	}
	// invalid machine/channel?	
	if ((note >= psy_audio_NOTECOMMANDS_MIDI_SPP && note <= psy_audio_NOTECOMMANDS_MIDI_SYNC) ||
			psy_audio_machines_at(&self->song->machines, busmachine)) {
		psy_audio_patternevent_init_all(rv, note, inst, busmachine,
			psy_audio_NOTECOMMANDS_VOL_EMPTY, cmd, parameter);
		return TRUE;
	} else {
		psy_audio_patternevent_init_all(rv, note, inst, busmachine,
			psy_audio_NOTECOMMANDS_VOL_EMPTY, cmd, parameter);
	}
	return FALSE;
}

void psy_audio_midiinput_setinstmap(psy_audio_MidiInput* self, uintptr_t channel,
	uintptr_t inst)
{
	assert(self);

	assert(channel < psy_audio_MAX_MIDI_CHANNELS);
	assert(inst < MAX_INSTRUMENTS);
	if (inst != self->channelinstmap[channel]) {
		self->channelinstmap[channel] = inst;
		++self->stats.channelmapupdate;
	}
}

uintptr_t psy_audio_midiinput_instmap(psy_audio_MidiInput* self, uintptr_t channel)
{
	assert(self);

	assert(channel < psy_audio_MAX_MIDI_CHANNELS);
	return self->channelinstmap[channel];
}

void psy_audio_midiinput_setgenmap(psy_audio_MidiInput* self, uintptr_t channel,
	uintptr_t generator)
{
	assert(self);

	assert(channel < psy_audio_MAX_MIDI_CHANNELS);
	// assert(generator < psy_audio_MAX_VIRTUALINSTS);
	if (generator != self->channelgeneratormap[channel]) {
		++self->stats.channelmapupdate;
		self->channelgeneratormap[channel] = generator;
	}
}

uintptr_t psy_audio_midiinput_genmap(const psy_audio_MidiInput* self, uintptr_t channel)
{
	assert(self);
	assert(channel < psy_audio_MAX_MIDI_CHANNELS);

	switch (self->midiconfig.gen_select_with) {
		case psy_audio_MIDICONFIG_MS_USE_SELECTED:
			if (self->song) {
				if (psy_audio_machines_at(&self->song->machines,
						psy_audio_machines_selected(&self->song->machines))) {
					return psy_audio_machines_selected(&self->song->machines);
				}
			}
			break;
		case psy_audio_MIDICONFIG_MS_MIDI_CHAN:
			if (self->song && psy_audio_machines_at(&self->song->machines,
					channel)) {
				return channel;
			}
			break;
		default:
			if (self->song && psy_audio_machines_at(&self->song->machines,
					self->channelgeneratormap[channel])) {
				return self->channelgeneratormap[channel];
			}
			break;
	}	
	return psy_INDEX_INVALID;
}

void psy_audio_setcontrollermap(psy_audio_MidiInput* self, int channel,
	int controller, int parameter)
{
	assert(channel < psy_audio_MAX_MIDI_CHANNELS);
	assert(controller < psy_audio_MAX_CONTROLLERS);
	assert(parameter < psy_audio_MAX_PARAMETERS);
	self->channelcontroller[channel][controller] = parameter;
}

int psy_audio_getcontrollermap(psy_audio_MidiInput* self, int channel, int controller)
{
	assert(channel < psy_audio_MAX_MIDI_CHANNELS);
	assert(controller < psy_audio_MAX_CONTROLLERS);
	return self->channelcontroller[channel][controller];
}

void psy_audio_midiinput_configure(psy_audio_MidiInput* self, psy_Property*
	configuration, bool datastr)
{
	assert(self);

	psy_audio_midiconfig_configure(&self->midiconfig, configuration, datastr);
}
