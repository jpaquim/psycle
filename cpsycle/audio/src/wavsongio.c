// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

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
	psy_audio_SampleIndex index;	
	
	sample = psy_audio_sample_allocinit(0);
	psy_audio_sample_load(sample, self->path);	
	index = index = psy_audio_sampleindex_make(0, 0);
	psy_audio_samples_insert(&self->song->samples, sample, index);
	instrument = psy_audio_instrument_allocinit();
	psy_audio_instrument_setname(instrument, psy_audio_sample_name(sample));
	psy_audio_instrument_setindex(instrument, index.slot);
	psy_audio_instruments_insert(&self->song->instruments, instrument,
		psy_audio_instrumentindex_make(0, index.slot));
	{
		psy_audio_Pattern* pattern;
		psy_audio_PatternEvent patternevent;		

		psy_audio_patternevent_clear(&patternevent);
		patternevent.note = 48;
		pattern = psy_audio_pattern_allocinit();
		psy_audio_pattern_insert(pattern, 0, 0, (psy_dsp_beat_t) 0, &patternevent);
		psy_audio_pattern_setlength(pattern,
			(sample->numframes / (psy_dsp_beat_t)sample->samplerate) / 60 * 125);
		psy_audio_patterns_insert(&self->song->patterns, 0, pattern);
		psy_audio_song_set_num_song_tracks(self->song, 1);
	}				
	psy_audio_sequence_append_track(&self->song->sequence,
		psy_audio_sequencetrack_allocinit());		
	psy_audio_sequence_insert(&self->song->sequence,
		psy_audio_orderindex_make(0, 0), 0);	
	{
		psy_audio_Machine* machine;

		machine = psy_audio_machinefactory_makemachine(
			self->song->machine_factory, psy_audio_SAMPLER, "",
			psy_INDEX_INVALID);
		if (machine) {
			psy_audio_machines_insert(&self->song->machines, 0, machine);
			psy_audio_machines_connect(&self->song->machines,
				psy_audio_wire_make(0, psy_audio_MASTER_INDEX));
		}
	}
}

void psy_audio_wav_songio_save(psy_audio_SongFile* self)
{

}


