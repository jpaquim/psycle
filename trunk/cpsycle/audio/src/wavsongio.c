// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "wavsongio.h"

#include "instrument.h"
#include "machinefactory.h"
#include "sample.h"
#include "song.h"
#include "songio.h"

#include <stdlib.h>
#include <string.h>

void psy_audio_wav_songio_load(psy_audio_SongFile* self)
{	
	psy_audio_Sample* sample;
	psy_audio_Instrument* instrument;	
	SampleIndex index;	
	
	sample = sample_allocinit();
	sample_load(sample, self->path);
	index = index = sampleindex_make(0, 0);
	psy_audio_samples_insert(&self->song->samples, sample, index);
	instrument = instrument_allocinit();
	instrument_setname(instrument, sample_name(sample));
	instruments_insert(&self->song->instruments, instrument,
		index.slot);	
	{
		psy_audio_Pattern* pattern;
		psy_audio_PatternEvent patternevent;		

		patternevent_clear(&patternevent);
		patternevent.note = 48;
		pattern = pattern_allocinit();
		pattern_insert(pattern, 0, 0, (psy_dsp_beat_t) 0, &patternevent);
		pattern_setlength(pattern, 
			(sample->numframes / (psy_dsp_beat_t)sample->samplerate) / 60 * 125);
		patterns_insert(&self->song->patterns, 0, pattern);
		patterns_setsongtracks(&self->song->patterns, 1);
	}
	{
		SequencePosition sequenceposition;
		
		sequenceposition.track = sequence_appendtrack(&self->song->sequence,
			sequencetrack_allocinit());
		sequenceposition.trackposition =
			sequence_begin(&self->song->sequence, sequenceposition.track, 0);
		sequence_insert(&self->song->sequence, sequenceposition, 0);
	}
	{
		psy_audio_Machine* machine;

		machine = machinefactory_makemachine(self->song->machinefactory,
			MACH_SAMPLER, "");
		if (machine) {			
			psy_Properties* machinesproperties;
			psy_Properties* machineproperties;
	
			machinesproperties = psy_properties_create_section(
				self->workspaceproperties, "machines");	
			machineproperties = psy_properties_create_section(
				machinesproperties, "machine");
			psy_properties_append_int(machineproperties, "index", 0, 0,
				256);
			psy_properties_append_int(machineproperties, "x", 100, 0, 0);
			psy_properties_append_int(machineproperties, "y", 100, 0, 0);
			self->workspaceproperties = machinesproperties;
			machines_insert(&self->song->machines, 0, machine);
			machines_connect(&self->song->machines, 0, MASTER_INDEX);
		}
	}
}

void psy_audio_wav_songio_save(psy_audio_SongFile* self)
{

}


