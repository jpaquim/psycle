// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "sequencer.h"

#include "machine.h"
#include "playertimeinfo.h"
#include "patternevent.h"
#include "patternsequence.h"
#include "player.h"
#include "song.h"

namespace psycle { namespace core {

Sequencer::Sequencer()
{
}

Sequencer::~Sequencer()
{
}

void Sequencer::set_player(Player& player)
{
	player_ = &player;
}

void Sequencer::Work(unsigned int nframes)
{
	///\todo: Need to add the events coming from the MIDI device. (Of course, first we need the MIDI device)
	///\todo this will not work frame correct for BPM changes for now
	double beats = nframes / ((time_info()->sampleRate() * 60) / time_info()->bpm());
	std::vector<PatternEvent*> events;
	song_->patternSequence().GetEventsInRange(time_info()->playBeatPos(), beats, events);
	unsigned int rest_frames = nframes;
	double last_pos = 0;
	for (std::vector<PatternEvent*>::iterator ev_it = events.begin();
		ev_it!= events.end() ; ++ev_it) {
		PatternEvent* ev = *ev_it;
		if (ev->IsGlobal()) {
			double pos = ev->time_offset();
			unsigned int num = static_cast<int>((pos -last_pos) * time_info()->samplesPerBeat());
			player_->process(num);
			rest_frames -= num;
			last_pos = pos;
			process_global_event(*ev);
		}
		execute_notes(ev->time_offset() - time_info()->playBeatPos(), *ev);
	}
	player_->process(rest_frames);
}

void Sequencer::process_global_event(const PatternEvent& event) {
	Machine::id_type mIndex;
	switch(event.command()) {
		case commandtypes::BPM_CHANGE:
			/*setBpm(event.parameter());
			if(loggers::trace()) {
				std::ostringstream s;
				s << "psycle: core: player: bpm change event found. position: " << timeInfo_.playBeatPos() << ", new bpm: " << event.parameter();
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}*/
			break;
		/*case commandtypes::JUMP_TO:
			//todo: fix this. parameter indicates the pattern, not the beat!
			timeInfo_.setPlayBeatPos(event.parameter());
			break;*/ // todo
		case commandtypes::SET_BYPASS:
			/*mIndex = event.target();
			if(mIndex < MAX_MACHINES && song().machine(mIndex) && song().machine(mIndex)->acceptsConnections()) //i.e. Effect
				song().machine(mIndex)->_bypass = event.parameter() != 0;*/
			break;
		case commandtypes::SET_MUTE:
			/*mIndex = event.target();
			if(mIndex < MAX_MACHINES && song().machine(mIndex))
				song().machine(mIndex)->_mute = event.parameter() != 0;*/
			break;
		case commandtypes::SET_VOLUME:
			/*if(event.machine() == 255) {
				Master & master(static_cast<Master&>(*song().machine(MASTER_INDEX)));
				master._outDry = static_cast<int>(event.parameter());
			} else {
				mIndex = event.machine();
				if(mIndex < MAX_MACHINES && song().machine(mIndex)) {
					Wire::id_type wire(event.target2());
					song().machine(mIndex)->SetDestWireVolume(mIndex, wire,
						value_mapper::map_255_1(static_cast<int>(event.parameter()))
					);
				}
			}*/
		break;
		case commandtypes::SET_PANNING:
			/*mIndex = event.target();
			if(mIndex < MAX_MACHINES && song().machine(mIndex))
				song().machine(mIndex)->SetPan(static_cast<int>( event.parameter()));
			break;*/
		default: ;
	}
}


void Sequencer::execute_notes(double beat_offset, PatternEvent& entry) {
	// WARNING!!! In this function, the events inside the patterline are assumed to be temporary! (thus, modifiable)

	int track = entry.track();
	int sequence_track = entry.sequence_track();

	int mac = entry.machine();
	if(mac != 255) prev_machines_[track] = mac;
	else mac = prev_machines_[track];

	// not a valid machine id?
	if(mac >= MAX_MACHINES || !song()->machine(mac))
		return;
	
	Machine& machine = *song()->machine(mac);

	// step 1: process all tweaks.
	{
		switch(entry.note()) {
			case notetypes::tweak_slide: {
				int const delay(64);
				int delaysamples(0), origin(machine.GetParamValue(entry.instrument()));
				float increment(origin);
				int previous(0);
				float rate = (((entry.command() << 16 ) | entry.parameter()) - origin) / (time_info()->samplesPerTick() / 64.0f);
					entry.setNote(notetypes::tweak);
					entry.setCommand(origin >> 8);
					entry.setParameter(origin & 0xff);
					machine.AddEvent(
						beat_offset + static_cast<double>(delaysamples) / time_info()->samplesPerBeat(),
						sequence_track * 1024 + track, entry
					);
					previous = origin;
					delaysamples += delay;
					while(delaysamples < time_info()->samplesPerTick()) {
						increment += rate;
						if(static_cast<int>(increment) != previous) {
							origin = static_cast<int>(increment);
							entry.setCommand(origin >> 8);
							entry.setParameter(origin & 0xff);
							machine.AddEvent(
								beat_offset + static_cast<double>(delaysamples) / time_info()->samplesPerBeat(),
								sequence_track * 1024 + track, entry
							);
							previous = origin;
						}
						delaysamples += delay;
					}
			} break;
			case notetypes::tweak:
				machine.AddEvent(beat_offset, sequence_track * 1024 + track, entry);
			break;
			default: 
			;
		}
	}

	// step 2: collect note
	{
		// track muted?
		if(song()->patternSequence().trackMuted(track)) return;

		// not a note ?
		if(entry.note() > notetypes::release && entry.note() != notetypes::empty) return;

		// machine muted?
		if(machine._mute) return;

		switch(entry.command()) {
			case commandtypes::NOTE_DELAY: {
				double delayoffset(entry.parameter() / 256.0);
				// At least Plucked String works erroneously if the command is not ommited.
				entry.setCommand(0); entry.setParameter(0);
				machine.AddEvent(beat_offset + delayoffset, sequence_track * 1024 + track, entry);
			} break;
			case commandtypes::RETRIGGER: {
				///\todo: delaysamples and rate should be memorized (for RETR_CONT command ). Then set delaysamples to zero in this function.
				int delaysamples(0);
				int rate = entry.parameter() + 1;
				int delay = (rate * static_cast<int>(time_info()->samplesPerTick())) >> 8;
				entry.setCommand(0); entry.setParameter(0);
				machine.AddEvent(beat_offset, sequence_track * 1024 + track, entry);
				delaysamples += delay;
				while(delaysamples < time_info()->samplesPerTick()) {
					machine.AddEvent(
					beat_offset + static_cast<double>(delaysamples) / time_info()->samplesPerBeat(),
					sequence_track * 1024 + track, entry
				);
				delaysamples += delay;
			}
		} break;
		case commandtypes::RETR_CONT: {
			///\todo: delaysamples and rate should be memorized, do not reinit delaysamples.
			///\todo: verify that using ints for rate and variation is enough, or has to be float.
			int delaysamples(0), rate(0), delay(0), variation(0);
			int parameter = entry.parameter() & 0x0f;
			variation = (parameter < 9) ? (4 * parameter) : (-2 * (16 - parameter));
			if(entry.parameter() & 0xf0) rate = entry.parameter() & 0xf0;
			delay = (rate * static_cast<int>(time_info()->samplesPerTick())) >> 8;
			entry.setCommand(0); entry.setParameter(0);
			machine.AddEvent(
				beat_offset + static_cast<double>(delaysamples) / time_info()->samplesPerBeat(),
				sequence_track * 1024 + track, entry
			);
			delaysamples += delay;
			while(delaysamples < time_info()->samplesPerTick()) {
				machine.AddEvent(
					beat_offset + static_cast<double>(delaysamples) / time_info()->samplesPerBeat(),
					sequence_track * 1024 + track, entry
				);
				rate += variation;
				if(rate < 16)
					rate = 16;
				delay = (rate * static_cast<int>(time_info()->samplesPerTick())) >> 8;
				delaysamples += delay;
			}
		} break;
		case commandtypes::ARPEGGIO: {
			///\todo : Add Memory.
			///\todo : This won't work... What about sampler's NNA's?
			#if 0
				if(entry.parameter()) {
					machine.TriggerDelay[track] = entry;
					machine.ArpeggioCount[track] = 1;
				}
				machine.RetriggerRate[track] = static_cast<int>(timeInfo_.samplesPerTick() * timeInfo_.linesPerBeat() / 24);
			#endif
		} break;
		default:
			machine.TriggerDelay[track].setCommand(0);
			machine.AddEvent(beat_offset, sequence_track * 1024 + track, entry);
			machine.TriggerDelayCounter[track] = 0;
			machine.ArpeggioCount[track] = 0;
		}
	}
}

}}
