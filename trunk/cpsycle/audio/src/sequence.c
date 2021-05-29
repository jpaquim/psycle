// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequence.h"
#include "sequencer.h" // calculate duration

#include "../../detail/portable.h"

// psy_audio_SequenceTrack
void psy_audio_sequencetrack_init(psy_audio_SequenceTrack* self)
{
	self->entries = NULL;
	self->name = psy_strdup("seq");	
}

void psy_audio_sequencetrack_dispose(psy_audio_SequenceTrack* self)
{
	psy_list_deallocate(&self->entries, (psy_fp_disposefunc)NULL);
	free(self->name);
	self->name = NULL;
}

psy_audio_SequenceTrack* psy_audio_sequencetrack_alloc(void)
{
	return malloc(sizeof(psy_audio_SequenceTrack));
}

psy_audio_SequenceTrack* psy_audio_sequencetrack_allocinit(void)
{
	psy_audio_SequenceTrack* rv;

	rv = psy_audio_sequencetrack_alloc();
	if (rv) {
		psy_audio_sequencetrack_init(rv);
	}
	return rv;
}

psy_audio_SequenceTrack* psy_audio_sequencetrack_clone(psy_audio_SequenceTrack* src)
{
	psy_audio_SequenceTrack* rv;

	assert(src);

	rv = psy_audio_sequencetrack_allocinit();
	if (rv) {
		psy_List* p;		
				
		for (p = src->entries; p != NULL; psy_list_next(&p)) {			
			psy_audio_SequenceEntry* entry;
			
			entry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (entry) {
				psy_list_append(&rv->entries, psy_audio_sequenceentry_clone(entry));
			} else {
				psy_list_append(&rv->entries, NULL);
			}
		}
		psy_strreset(&rv->name, src->name);
	}
	return rv;
}

psy_dsp_big_beat_t psy_audio_sequencetrack_duration(psy_audio_SequenceTrack* self,
	psy_audio_Patterns* patterns)
{
	psy_dsp_big_beat_t rv;
	psy_List* p;

	assert(self);

	rv = 0.0;
	p = self->entries;
	if (p) {
		psy_audio_Pattern* pattern;
		psy_audio_SequenceEntry* entry;

		p = p->tail;
		entry = (psy_audio_SequenceEntry*)p->entry;
		if (entry) {
			pattern = psy_audio_patterns_at(patterns, entry->patternslot);
			if (pattern) {
				if (rv < entry->offset + pattern->length) {
					rv = entry->offset + pattern->length;
				}
			}
		}
	}
	return rv;
}

void psy_audio_sequencetrack_setname(psy_audio_SequenceTrack* self, const char* name)
{
	assert(self);

	self->name = psy_strreset(&self->name, name);
}


// psy_audio_SequenceEntry
void psy_audio_sequenceentry_init(psy_audio_SequenceEntry* self, uintptr_t patternslot,
	psy_dsp_big_beat_t offset)
{
	self->patternslot = patternslot;
	self->sampleindex = psy_audio_sampleindex_zero();
	self->offset = offset;
	self->repositionoffset = 0.0;
	self->selplay = 0;	
	self->row = 0;
}

psy_audio_SequenceEntry* psy_audio_sequenceentry_alloc(void)
{
	return (psy_audio_SequenceEntry*)malloc(sizeof(psy_audio_SequenceEntry));
}

psy_audio_SequenceEntry* psy_audio_sequenceentry_allocinit(uintptr_t pattern, psy_dsp_big_beat_t offset)
{
	psy_audio_SequenceEntry* rv;

	rv = psy_audio_sequenceentry_alloc();
	if (rv) {
		psy_audio_sequenceentry_init(rv, pattern, offset);
	}
	return rv;
}

psy_audio_SequenceEntry* psy_audio_sequenceentry_clone(psy_audio_SequenceEntry* src)
{
	psy_audio_SequenceEntry* rv;

	rv = psy_audio_sequenceentry_alloc();
	if (rv) {
		psy_audio_sequenceentry_init(rv, src->patternslot, src->offset);
		rv->repositionoffset = src->repositionoffset;
		rv->row = src->row;
		rv->selplay = src->selplay;		
	}
	return rv;
}

// psy_audio_SequencePosition
void psy_audio_sequenceposition_init(psy_audio_SequencePosition* self)
{
	assert(self);

	self->tracknode = NULL;
	self->trackposition.patternnode = NULL;
	self->trackposition.sequencentrynode = NULL;
}

psy_audio_SequenceEntry* psy_audio_sequenceposition_entry(psy_audio_SequencePosition* self)
{
	psy_audio_SequenceEntry* rv = 0;

	assert(self);

	if (self->trackposition.sequencentrynode) {
		rv = (psy_audio_SequenceEntry*)self->trackposition.sequencentrynode->entry;
	}
	return rv;
}

// Sequence
static void psy_audio_sequence_initsignals(psy_audio_Sequence*);
static void psy_audio_sequence_disposesignals(psy_audio_Sequence*);
static void sequence_onpatternlengthchanged(psy_audio_Sequence* self,
	psy_audio_Pattern* sender);
static void sequence_onpatternnamechanged(psy_audio_Sequence* self,
	psy_audio_Pattern* sender);

void psy_audio_sequence_init(psy_audio_Sequence* self, psy_audio_Patterns* patterns)
{
	psy_audio_sequence_initsignals(self);
	self->tracks = 0;
	self->patterns = patterns;
	self->preventreposition = FALSE;
	self->sequencerduration = NULL;
	psy_audio_trackstate_init(&self->trackstate);
}

void psy_audio_sequence_initsignals(psy_audio_Sequence* self)
{
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_clear);
	psy_signal_init(&self->signal_insert);
	psy_signal_init(&self->signal_remove);
	psy_signal_init(&self->signal_reorder);
	psy_signal_init(&self->signal_trackinsert);
	psy_signal_init(&self->signal_trackremove);
	psy_signal_init(&self->signal_trackswap);
	psy_signal_init(&self->signal_trackreposition);
	psy_signal_init(&self->signal_solochanged);
	psy_signal_init(&self->signal_mutechanged);
}

void psy_audio_sequence_dispose(psy_audio_Sequence* self)
{
	psy_list_deallocate(&self->tracks, (psy_fp_disposefunc)
		psy_audio_sequencetrack_dispose);
	psy_audio_trackstate_dispose(&self->trackstate);	
	psy_audio_sequence_disposesignals(self);	
	if (self->sequencerduration) {
		psy_audio_sequencer_dispose(self->sequencerduration);
		free(self->sequencerduration);
		self->sequencerduration = NULL;
	}
}

void psy_audio_sequence_disposesignals(psy_audio_Sequence* self)
{
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_clear);
	psy_signal_dispose(&self->signal_insert);
	psy_signal_dispose(&self->signal_remove);
	psy_signal_dispose(&self->signal_trackswap);
	psy_signal_dispose(&self->signal_reorder);
	psy_signal_dispose(&self->signal_trackinsert);
	psy_signal_dispose(&self->signal_trackremove);
	psy_signal_dispose(&self->signal_trackreposition);
	psy_signal_dispose(&self->signal_solochanged);
	psy_signal_dispose(&self->signal_mutechanged);
}

void psy_audio_sequence_copy(psy_audio_Sequence* self, psy_audio_Sequence* other)
{
	psy_List* t;

	psy_audio_sequence_clear(self);
	for (t = other->tracks; t != NULL; psy_list_next(&t)) {
		psy_audio_SequenceTrack* srctrack;

		srctrack = (psy_audio_SequenceTrack*)psy_list_entry(t);
		psy_audio_sequence_appendtrack(self, psy_audio_sequencetrack_clone(srctrack));
	}
	psy_audio_trackstate_copy(&self->trackstate, &other->trackstate);	
	self->patterns = other->patterns;	
}

void psy_audio_sequence_clear(psy_audio_Sequence* self)
{
	psy_list_deallocate(&self->tracks, (psy_fp_disposefunc)
		psy_audio_sequencetrack_dispose);
	psy_audio_trackstate_dispose(&self->trackstate);
	psy_audio_trackstate_init(&self->trackstate);
	psy_signal_emit(&self->signal_clear, self, 0);
}

void psy_audio_sequence_reposition_track(psy_audio_Sequence* self, psy_audio_SequenceTrack* track)
{
	if (!self->preventreposition) {
		psy_dsp_big_beat_t curroffset;	
		psy_List* p;
		uintptr_t row;
		uintptr_t trackidx;
			
		for (p = track->entries, row = 0, curroffset = 0.0; p != NULL;
				psy_list_next(&p), ++row) {
			psy_audio_Pattern* pattern;
			psy_audio_SequenceEntry* sequenceentry;
		
			sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (sequenceentry) {
				sequenceentry->row = row;
				pattern = psy_audio_patterns_at(self->patterns,
					psy_audio_sequenceentry_patternslot(sequenceentry));
				if (pattern) {
					sequenceentry->offset = curroffset + sequenceentry->repositionoffset;
					curroffset = sequenceentry->offset + pattern->length;
				} else {
					sequenceentry->offset = curroffset + sequenceentry->repositionoffset;
				}
			}
		}	
		trackidx = psy_list_entry_index(self->tracks, track);
		psy_signal_emit(&self->signal_trackreposition, self, 1, trackidx);
	}
}

void psy_audio_reposition(psy_audio_Sequence* self)
{
	psy_audio_SequenceTrackNode* t;

	for (t = self->tracks; t != NULL; psy_list_next(&t)) {
		psy_audio_sequence_reposition_track(self, (psy_audio_SequenceTrack*)(t->entry));
	}
}

psy_audio_SequencePosition psy_audio_sequence_at(psy_audio_Sequence* self,
	uintptr_t trackindex, uintptr_t position)
{
	psy_audio_SequencePosition rv;	
	psy_List* ptr;	
	uintptr_t c = 0;
	psy_audio_SequenceTrackNode* ptracks;
	psy_audio_SequenceTrack* track;

	psy_audio_sequenceposition_init(&rv);	
	rv.trackposition.patterns = self->patterns;
	ptracks = self->tracks;
	while (ptracks) {
		if (c == trackindex) {
			break;
		}
		ptracks = ptracks->next;
		++c;
	}

	if (ptracks) {
		track = (psy_audio_SequenceTrack*)(ptracks->entry);
	} else {
		track = 0;
	}
	
	rv.tracknode = ptracks;
	if (rv.tracknode) {
		ptr = track->entries;
		c = 0;
		while (ptr) {
			if (c == position) {			
				rv = psy_audio_sequence_makeposition(self, ptracks, ptr);
				break;
			}
			++c;
			ptr = ptr->next;
		}
	}
	return rv;
}

psy_List* sequenceentry_at_offset(psy_audio_Sequence* self,
	psy_audio_SequenceTrackNode* tracknode, psy_dsp_big_beat_t offset)
{
	psy_dsp_big_beat_t curroffset = 0.0f;	
	psy_List* p = 0;

	if (tracknode) {		
		psy_audio_SequenceTrack* track;

		track = (psy_audio_SequenceTrack*)tracknode->entry;
		p = track->entries;	
		while (p) {
			psy_audio_Pattern* pattern;
			psy_audio_SequenceEntry* entry;
			
			entry = (psy_audio_SequenceEntry*)p->entry;
			if (entry) {
				pattern = psy_audio_patterns_at(self->patterns, entry->patternslot);
				if (pattern) {
					if (offset >= curroffset && offset < curroffset + pattern->length) {
						break;
					}
					curroffset += pattern->length;
				}
			}
			psy_list_next(&p);
		}
	}
	return p;
}

psy_audio_SequenceTrackIterator psy_audio_sequence_begin(
	psy_audio_Sequence* self, psy_List* track, psy_dsp_big_beat_t position)
{		
	psy_audio_SequenceTrackIterator rv;		
	psy_audio_SequenceEntry* entry;	

	rv.patterns = self->patterns;
	rv.sequencentrynode = sequenceentry_at_offset(self, track, position);
	if (rv.sequencentrynode) {
		entry = (psy_audio_SequenceEntry*)rv.sequencentrynode->entry;
		if (entry) {
			rv.pattern = psy_audio_patterns_at(self->patterns, entry->patternslot);
			if (rv.pattern) {
				rv.patternnode = psy_audio_pattern_greaterequal(rv.pattern,
					position - entry->offset);
			} else {
				rv.patternnode = NULL;
			}
		} else {
			rv.pattern = NULL;
			rv.patternnode = NULL;
		}
	} else {
		rv.pattern = NULL;
		rv.patternnode = NULL;
	}
	return rv;	
}

void psy_audio_sequencetrackiterator_inc(psy_audio_SequenceTrackIterator* self)
{	
	if (self->patternnode) {		
		self->patternnode = self->patternnode->next;
		if (self->patternnode == NULL && self->sequencentrynode) {
			if (self->sequencentrynode->next) {
				psy_audio_SequenceEntry* entry;
				psy_audio_Pattern* pattern;
				self->sequencentrynode = self->sequencentrynode->next;
				entry = (psy_audio_SequenceEntry*)self->sequencentrynode->entry;
				if (entry) {
					pattern = psy_audio_patterns_at(self->patterns, entry->patternslot);
					if (pattern) {
						self->patternnode = pattern->events;
					} else {
						self->patternnode = NULL;
					}
				} else {
					self->patternnode = NULL;
				}
			}
		}
	}
}

void psy_audio_sequencetrackiterator_inc_entry(psy_audio_SequenceTrackIterator* self)
{	
	if (self->sequencentrynode) {
		self->sequencentrynode = self->sequencentrynode->next;
		if (self->sequencentrynode) {
			psy_audio_SequenceEntry* entry;			

			entry = (psy_audio_SequenceEntry*) self->sequencentrynode->entry;
			if (entry) {
				self->pattern = psy_audio_patterns_at(self->patterns, entry->patternslot);
				if (self->pattern) {
					self->patternnode = self->pattern->events;
				} else {
					self->patternnode = 0;
				}
			} else {
				self->patternnode = 0;
			}
		} else {
			self->patternnode = 0;
		}
	}
}

void psy_audio_sequencetrackiterator_dec_entry(psy_audio_SequenceTrackIterator* self)
{	
	if (self->sequencentrynode) {
		self->sequencentrynode = self->sequencentrynode->prev;
		if (self->sequencentrynode) {
			psy_audio_SequenceEntry* entry;

			entry = (psy_audio_SequenceEntry*) self->sequencentrynode->entry;
			if (entry) {
				self->pattern = psy_audio_patterns_at(self->patterns, entry->patternslot);
				self->patternnode = self->pattern->events;
			} else {
				self->pattern = NULL;
				self->patternnode = NULL;
			}
		} else {
			self->patternnode = 0;
		}
	}
}

psy_audio_SequenceTrackIterator sequence_makeiterator(psy_audio_Sequence* self, psy_List* entries)
{
	psy_audio_SequenceTrackIterator rv;
	psy_audio_Pattern* pPattern  = 0;	
	
	rv.patterns = self->patterns;
	rv.sequencentrynode = entries;
	if (entries) {
		psy_audio_SequenceEntry* entry = (psy_audio_SequenceEntry*)entries->entry;
		if (entry) {
			pPattern = psy_audio_patterns_at(self->patterns, entry->patternslot);
			if (pPattern) {
				rv.patternnode = pPattern->events;
			} else {
				rv.patternnode = 0;
			}
		} else {
			rv.patternnode = NULL;
			rv.pattern = NULL;
		}
	}		
	return rv;
}

psy_audio_SequencePosition psy_audio_sequence_makeposition(psy_audio_Sequence* self,
	psy_audio_SequenceTrackNode* track, psy_List* entries)
{
	psy_audio_SequencePosition rv;

	rv.trackposition = sequence_makeiterator(self, entries);
	rv.tracknode = track;
	return rv;
}

void psy_audio_sequence_appendtrack(psy_audio_Sequence* self,
	psy_audio_SequenceTrack* track)
{	
	assert(self);

	psy_list_append(&self->tracks, track);
	psy_signal_emit(&self->signal_trackinsert, self, 1,
		psy_list_size(track->entries) - 1);
	psy_signal_emit(&self->signal_changed, self, 0);
}

psy_audio_SequenceTrackNode* psy_audio_sequence_removetrack(psy_audio_Sequence*
	self, psy_audio_SequenceTrackNode* tracknode)
{	
	psy_List* rv;
	uintptr_t index;

	assert(self);
	assert(tracknode);

	index = psy_list_entry_index(self->tracks, tracknode->entry);
	rv = psy_list_remove(&self->tracks, tracknode);
	psy_signal_emit(&self->signal_trackremove, self, 1, index);
	psy_signal_emit(&self->signal_changed, self, 0);
	return rv;
}

psy_audio_SequenceTrack* psy_audio_sequence_track_at(psy_audio_Sequence* self,
	uintptr_t index)
{
	psy_audio_SequenceTrackNode* t;

	assert(self);

	t = psy_list_at(self->tracks, index);
	if (t) {
		return (psy_audio_SequenceTrack*)psy_list_entry(t);
	}
	return NULL;
}

const psy_audio_SequenceTrack* psy_audio_sequence_track_at_const(const
	psy_audio_Sequence* self, uintptr_t index)
{
	assert(self);

	return psy_audio_sequence_track_at((psy_audio_Sequence*)self, index);
}

uintptr_t psy_audio_sequence_width(const psy_audio_Sequence* self)
{
	return psy_list_size(self->tracks);	
}

bool psy_audio_sequence_patternused(psy_audio_Sequence* self, uintptr_t patternslot)
{
	int rv = FALSE;
	psy_audio_SequenceTrackNode* t;

	assert(self);

	t = self->tracks;
	while (t) {
		psy_audio_SequenceTrack* track;
		psy_audio_SequenceEntryNode* p;

		track = (psy_audio_SequenceTrack*)t->entry;
		p = track->entries;
		while (p) {
			psy_audio_SequenceEntry* sequenceentry;

			sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (sequenceentry) {
				if (psy_audio_sequenceentry_patternslot(sequenceentry) ==
					patternslot) {
					rv = TRUE;
					break;
				}
			}
			psy_list_next(&p);
		}
		psy_list_next(&t);
	}
	return rv;
}

psy_audio_OrderIndex psy_audio_sequence_patternfirstused(psy_audio_Sequence* self,
	uintptr_t patternslot)
{
	psy_audio_OrderIndex rv;
	psy_audio_SequenceTrackNode* t;
	uintptr_t currtrackidx;

	assert(self);
	rv.track = psy_INDEX_INVALID;
	rv.order = psy_INDEX_INVALID;
	
	t = self->tracks;
	currtrackidx = 0;
	while (t) {
		psy_audio_SequenceTrack* track;
		psy_audio_SequenceEntryNode* p;
		uintptr_t currorderidx;

		track = (psy_audio_SequenceTrack*)t->entry;
		currorderidx = 0;
		p = track->entries;
		while (p) {
			psy_audio_SequenceEntry* sequenceentry;

			sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (sequenceentry && psy_audio_sequenceentry_patternslot(sequenceentry) ==
					patternslot) {
				psy_audio_Pattern* pattern;

				pattern = psy_audio_patterns_at(self->patterns, patternslot);
				rv.track = currtrackidx;
				rv.order = currorderidx;
				break;
			}
			psy_list_next(&p);
			++currorderidx;
		}
		psy_list_next(&t);
		++currtrackidx;
	}
	return rv;
}

psy_dsp_big_beat_t psy_audio_sequence_duration(psy_audio_Sequence* self)
{	
	psy_audio_SequenceTrackNode* t;
	psy_dsp_big_beat_t rv;

	assert(self);

	rv = 0.0;
	t = self->tracks;	
	for (t = self->tracks; t != NULL; psy_list_next(&t)) {
		psy_audio_SequenceTrack* track;
		psy_dsp_big_beat_t trackduration;

		track = (psy_audio_SequenceTrack*)t->entry;
		trackduration = psy_audio_sequencetrack_duration(track,
			self->patterns);		
		if (rv < trackduration) {
			rv = trackduration;
		}
	}
	return rv;
}

uintptr_t psy_audio_sequence_maxtracksize(psy_audio_Sequence* self)
{
	uintptr_t rv = 0;
	psy_audio_SequenceTrackNode* t;
	
	assert(self);

	for (t = self->tracks; t != NULL; psy_list_next(&t)) {
		psy_audio_SequenceTrack* track;
		uintptr_t tracksize;

		track = (psy_audio_SequenceTrack*)psy_list_entry(t);
		tracksize = psy_list_size(track->entries);
		if (tracksize > rv) {
			rv = tracksize;
		}
	}
	return rv;
}

void psy_audio_sequence_setplayselection(psy_audio_Sequence* self,
	psy_audio_SequenceSelection* selection)
{
	psy_List* p;

	psy_audio_sequence_clearplayselection(self);
	for (p = selection->entries; p != NULL; psy_list_next(&p)) {
		psy_audio_OrderIndex* index;
		psy_audio_SequenceEntry* entry;

		index = (psy_audio_OrderIndex*)psy_list_entry(p);		
		entry = psy_audio_sequence_entry(self, *index);
		if (entry) {
			entry->selplay = TRUE;
		}
	}
}

void psy_audio_sequence_clearplayselection(psy_audio_Sequence* self)
{
	psy_audio_SequenceTrackNode* t;
	
	for (t = self->tracks; t != 0; t = t->next) {
		psy_audio_SequenceTrack* track;
		psy_List* p;

		track = t->entry;
		for (p = track->entries; p != NULL; psy_list_next(&p)) {
			psy_audio_SequenceEntry* entry;

			entry = (psy_audio_SequenceEntry*)p->entry;
			if (entry) {
				entry->selplay = FALSE;
			}
		}
	}
}

void psy_audio_sequence_startcalcdurationinms(psy_audio_Sequence* self)
{
	self->durationms = 0.0;
	if (!self->sequencerduration) {
		self->sequencerduration =
			(psy_audio_Sequencer*)malloc(sizeof(psy_audio_Sequencer));
	}
	if (!self->sequencerduration) {
		return;
	}
	psy_audio_sequencer_init(self->sequencerduration, self, NULL);
	psy_audio_sequencer_stoploop(self->sequencerduration);
	psy_audio_sequencer_start(self->sequencerduration);
	self->sequencerduration->calcduration = TRUE;
}

psy_dsp_big_seconds_t psy_audio_sequence_endcalcdurationinmsresult(psy_audio_Sequence* self)
{	
	if (self->sequencerduration) {
		psy_dsp_big_seconds_t rv;

		rv = psy_audio_sequencer_currplaytime(self->sequencerduration);
		psy_audio_sequencer_dispose(self->sequencerduration);
		free(self->sequencerduration);
		self->sequencerduration = NULL;
		return rv;
	}
	return 0.0;
}

bool psy_audio_sequence_calcdurationinms(psy_audio_Sequence* self)
{		
	uintptr_t maxamount;
	uintptr_t amount;
	uintptr_t numsamplex;

	if (!self->sequencerduration) {
		return FALSE;
	}		
	numsamplex = 8192; // psy_audio_MAX_STREAM_SIZE;
	maxamount = numsamplex;		
	do {
		amount = maxamount;
		if (amount > numsamplex) {
			amount = numsamplex;
		}
		if (self->sequencerduration->linetickcount <=
			psy_audio_sequencer_frametooffset(self->sequencerduration, amount)) {
			if (self->sequencerduration->linetickcount > 0) {
				uintptr_t pre;

				pre = psy_audio_sequencer_frames(self->sequencerduration,
					self->sequencerduration->linetickcount);
				if (pre) {
					pre--;
					if (pre) {
						psy_audio_sequencer_frametick(self->sequencerduration, pre);
						numsamplex -= pre;
						amount -= pre;
						self->sequencerduration->linetickcount -=
							psy_audio_sequencer_frametooffset(
								self->sequencerduration, pre);
					}
				}
			}					
			psy_audio_sequencer_onnewline(self->sequencerduration);
		}			
		if (amount > 0) {
			psy_audio_sequencer_frametick(self->sequencerduration, amount);
			numsamplex -= amount;
			self->sequencerduration->linetickcount -=
				psy_audio_sequencer_frametooffset(self->sequencerduration,
					amount);
		}
	} while (numsamplex > 0);
	return (psy_audio_sequencer_playing(self->sequencerduration));	
}

void sequence_onpatternlengthchanged(psy_audio_Sequence* self,
	psy_audio_Pattern* sender)
{
	psy_audio_reposition(self);
	psy_signal_emit(&self->signal_changed, self, 0);
}

void sequence_onpatternnamechanged(psy_audio_Sequence* self,
	psy_audio_Pattern* sender)
{
	psy_signal_emit(&self->signal_changed, self, 0);
}

// TrackState
void psy_audio_sequence_activatesolotrack(psy_audio_Sequence* self,
	uintptr_t track)
{
	assert(self);

	psy_audio_trackstate_activatesolotrack(&self->trackstate, track);
	psy_signal_emit(&self->signal_solochanged, self, 1, track);
}

void psy_audio_sequence_deactivatesolotrack(psy_audio_Sequence* self)
{
	assert(self);
	uintptr_t soloedtrack;
	
	soloedtrack = self->trackstate.soloedtrack;	
	psy_audio_trackstate_deactivatesolotrack(&self->trackstate);
	psy_signal_emit(&self->signal_solochanged, self, 1, soloedtrack);
}

void psy_audio_sequence_mutetrack(psy_audio_Sequence* self, uintptr_t track)
{
	assert(self);

	psy_audio_trackstate_mutetrack(&self->trackstate, track);
	psy_signal_emit(&self->signal_mutechanged, self, 1, track);
}

void psy_audio_sequence_unmutetrack(psy_audio_Sequence* self, uintptr_t track)
{
	assert(self);

	psy_audio_trackstate_unmutetrack(&self->trackstate, track);
	psy_signal_emit(&self->signal_mutechanged, self, 1, track);
}

int psy_audio_sequence_istrackmuted(const psy_audio_Sequence* self, uintptr_t track)
{
	assert(self);

	return psy_audio_trackstate_istrackmuted(&self->trackstate, track);
}

int psy_audio_sequence_istracksoloed(const psy_audio_Sequence* self, uintptr_t track)
{
	assert(self);

	return psy_audio_trackstate_istracksoloed(&self->trackstate, track);
}

// SequenceSelection
void psy_audio_sequenceselection_init(psy_audio_SequenceSelection* self)
{		
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_clear);
	psy_signal_init(&self->signal_select);
	psy_signal_init(&self->signal_deselect);
	psy_signal_init(&self->signal_update);
	self->entries = NULL;
	self->mode = psy_audio_SEQUENCESELECTION_SINGLE;
	psy_audio_sequenceselection_seteditposition(
		self, psy_audio_orderindex_make(0, 0));	
}

void psy_audio_sequenceselection_dispose(psy_audio_SequenceSelection* self)
{
	psy_list_deallocate(&self->entries, NULL);
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_clear);
	psy_signal_dispose(&self->signal_select);
	psy_signal_dispose(&self->signal_deselect);
	psy_signal_dispose(&self->signal_update);
}

void psy_audio_sequenceselection_clear(psy_audio_SequenceSelection* self)
{
	psy_list_deallocate(&self->entries, NULL);
	psy_signal_emit(&self->signal_clear, self, 0);
	psy_signal_emit(&self->signal_changed, self, 0);
}

void psy_audio_sequenceselection_copy(psy_audio_SequenceSelection* self,
	psy_audio_SequenceSelection* other)
{
	psy_List* p;

	psy_list_deallocate(&self->entries, NULL);
	self->mode = other->mode;	
	for (p = other->entries; p != NULL; psy_list_next(&p)) {
		psy_audio_OrderIndex* currindex;
		psy_audio_OrderIndex* newindex;

		currindex = (psy_audio_OrderIndex*)psy_list_entry(p);
		newindex = (psy_audio_OrderIndex*)
			malloc(sizeof(psy_audio_OrderIndex));
		if (newindex) {
			*newindex = *currindex;
			psy_list_append(&self->entries, newindex);
		}		
	}
}

void psy_audio_sequenceselection_seteditposition(
	psy_audio_SequenceSelection* self,
	psy_audio_OrderIndex index)
{
	self->editposition = index;
	psy_audio_sequenceselection_select(self, index);	
}

void psy_audio_sequenceselection_select(psy_audio_SequenceSelection* self,
	psy_audio_OrderIndex index)
{
	psy_audio_OrderIndex* newindex;

	if (self->mode == psy_audio_SEQUENCESELECTION_SINGLE) {
		if (self->entries) {
			psy_audio_OrderIndex temp;

			temp = *((psy_audio_OrderIndex*)self->entries->entry);
			psy_list_deallocate(&self->entries, NULL);
			psy_signal_emit(&self->signal_deselect, self, 1, &temp);			
		}
	} else if (self->mode == psy_audio_SEQUENCESELECTION_MULTI) {
		if (psy_audio_sequenceselection_isselected(self, index)) {
			psy_audio_sequenceselection_deselect(self, index);
			if (self->entries) {
				psy_signal_emit(&self->signal_deselect, self, 1, &index);
				psy_signal_emit(&self->signal_changed, self, 0);
				return;
			}
			index = self->editposition;
		}
	}
	newindex = (psy_audio_OrderIndex*)
		malloc(sizeof(psy_audio_OrderIndex));
	if (newindex) {
		*newindex = index;
		psy_list_append(&self->entries, newindex);
	}
	psy_signal_emit(&self->signal_select, self, 1, &index);
	psy_signal_emit(&self->signal_changed, self, 0);
}

void psy_audio_sequenceselection_deselect(psy_audio_SequenceSelection* self,
	psy_audio_OrderIndex index)
{
	psy_List* p;

	p = self->entries;
	while (p) {
		psy_audio_OrderIndex* currindex;

		currindex = (psy_audio_OrderIndex*)psy_list_entry(p);
		if (currindex->order == index.order && currindex->track == index.track) {
			free(p->entry);
			psy_list_remove(&self->entries, p);
			break;
		}
		psy_list_next(&p);
	}	
}

bool psy_audio_sequenceselection_isselected(const psy_audio_SequenceSelection* self,
	psy_audio_OrderIndex index)
{
	psy_List* p;

	p = self->entries;
	while (p) {
		psy_audio_OrderIndex* currindex;

		currindex = (psy_audio_OrderIndex*)psy_list_entry(p);
		if (currindex->order == index.order && currindex->track == index.track) {
			break;
		}
		psy_list_next(&p);
	}
	return p != NULL;
}

psy_audio_OrderIndex psy_audio_sequenceselection_first(const psy_audio_SequenceSelection* self)
{
	if (self->entries) {
		return *((psy_audio_OrderIndex*)self->entries->entry);
	}
	return psy_audio_orderindex_zero();
}

void psy_audio_sequenceselection_setmode(psy_audio_SequenceSelection* self,
	psy_audio_SequenceSelectionMode mode)
{
	self->mode = mode;
	switch (self->mode) {
		case psy_audio_SEQUENCESELECTION_MULTI:
			break;
		// fallthrough
		case psy_audio_SEQUENCESELECTION_SINGLE:
		default:			
			psy_audio_sequenceselection_select(self, self->editposition);
			break;
	}
}

void psy_audio_sequenceselection_update(psy_audio_SequenceSelection* self)
{
	psy_signal_emit(&self->signal_update, self, 0);
	psy_signal_emit(&self->signal_changed, self, 0);
}

// SequencePaste

void psy_audio_sequencepaste_init(psy_audio_SequencePaste* self)
{
	self->entries = NULL;	
}

void psy_audio_sequencepaste_dispose(psy_audio_SequencePaste* self)
{
	psy_list_deallocate(&self->entries, (psy_fp_disposefunc)NULL);
}

void psy_audio_sequencepaste_clear(psy_audio_SequencePaste* self)
{
	psy_list_deallocate(&self->entries, (psy_fp_disposefunc)NULL);
}

void psy_audio_sequencepaste_copy(psy_audio_SequencePaste* self,
	psy_audio_Sequence* sequence, psy_audio_SequenceSelection* selection)
{
	psy_List* p;	
	psy_audio_OrderIndex minindex;

	psy_audio_sequencepaste_clear(self);
	minindex.order = psy_INDEX_INVALID;
	minindex.track = psy_INDEX_INVALID;
	for (p = selection->entries; p != NULL; psy_list_next(&p)) {
		psy_audio_OrderIndex* index;
		psy_audio_SequenceEntry* entry;
		psy_audio_Order* order;

		index = (psy_audio_OrderIndex*)psy_list_entry(p);
		assert(index);
		entry = psy_audio_sequence_entry(sequence, *index);
		if (entry) {
			order = (psy_audio_Order*)malloc(sizeof(psy_audio_Order));
			if (order) {
				order->index = *index;
				if (order->index.order < minindex.order) {
					minindex.order = order->index.order;
				}
				if (order->index.track < minindex.track) {
					minindex.track = order->index.track;
				}
				order->entry = *entry;
				psy_list_append(&self->entries, order);
			}
		}
	}
	for (p = self->entries; p != NULL; psy_list_next(&p)) {
		psy_audio_Order* order;

		order = (psy_audio_Order*)psy_list_entry(p);
		if (psy_audio_orderindex_valid(&order->index)) {
			order->index.order -= minindex.order;
			order->index.track -= minindex.track;
		}
	}
}


// Sequence
void psy_audio_sequence_insert(psy_audio_Sequence* self,
	psy_audio_OrderIndex index, uintptr_t patidx)
{
	psy_audio_SequenceTrack* track;

	assert(self);

	track = psy_audio_sequence_track_at(self, index.track);
	if (track) {
		psy_List* p;
		psy_audio_SequenceEntry* entry;

		p = psy_list_at(track->entries, index.order);
		if (!p) {
			p = psy_list_last(track->entries);
		}
		entry = psy_audio_sequenceentry_allocinit(patidx,
			(psy_dsp_big_beat_t)0.0);
		psy_list_insert(&track->entries, p, entry);
		psy_audio_sequence_reposition_track(self, track);
		if (self->patterns) {
			psy_audio_Pattern* pattern;

			pattern = psy_audio_patterns_at(self->patterns, patidx);
			if (pattern) {
				psy_signal_connect(&pattern->signal_lengthchanged,
					self, sequence_onpatternlengthchanged);
				psy_signal_connect(&pattern->signal_namechanged,
					self, sequence_onpatternnamechanged);
			}
		}
		psy_signal_emit(&self->signal_insert, self, 1, &index);
		psy_signal_emit(&self->signal_changed, self, 0);
	}
}

void psy_audio_sequence_remove(psy_audio_Sequence* self,
	psy_audio_OrderIndex index)
{
	psy_audio_SequenceTrack* track;

	assert(self);

	track = psy_audio_sequence_track_at(self, index.track);
	if (track) {
		psy_List* p;

		p = psy_list_at(track->entries, index.order);
		if (p) {
			psy_audio_SequenceEntry* entry;
			uintptr_t patidx;

			entry = (psy_audio_SequenceEntry*)p->entry;
			if (entry) {
				patidx = entry->patternslot;
				psy_list_remove(&track->entries, p);
				free(entry);
				entry = NULL;
				psy_audio_sequence_reposition_track(self, track);
				if (self->patterns &&
					!psy_audio_sequence_patternused(self, patidx)) {
					psy_audio_Pattern* pattern;

					pattern = psy_audio_patterns_at(self->patterns, patidx);
					if (pattern) {
						psy_signal_disconnect_context(&pattern->signal_lengthchanged, self);
						psy_signal_disconnect_context(&pattern->signal_namechanged, self);
					}
				}
			}
			psy_signal_emit(&self->signal_remove, self, 1, &index);
			psy_signal_emit(&self->signal_changed, self, 0);
		}
	}
}

void psy_audio_sequence_remove_selection(psy_audio_Sequence* self,
	psy_audio_SequenceSelection* selection)
{
	psy_audio_SequenceSelectionIterator ite;
	psy_audio_SequenceTrackNode* t;
	bool removed;
	uintptr_t c;
	uintptr_t trackidx;

	assert(self);
	
	removed = FALSE;
	ite = psy_audio_sequenceselection_begin(selection);
	for (; ite != NULL; psy_list_next(&ite)) {
		psy_audio_OrderIndex* orderindex;
		psy_audio_SequenceEntryNode* currnode;
		psy_audio_SequenceTrack* track;

		orderindex = ite->entry;
		assert(orderindex);
		track = NULL;
		currnode = psy_audio_sequence_node(self, *orderindex, &track);
		if (currnode && track) {
			free(currnode->entry);
			currnode->entry = NULL;
		}		
	}
	trackidx = 0;
	for (t = self->tracks; t != NULL; psy_list_next(&t), ++trackidx) {
		psy_audio_SequenceTrack* track;
		psy_List* p;
		c = 0;

		track = (psy_audio_SequenceTrack*)psy_list_entry(t);
		p = track->entries;		
		while (p != NULL) {
			psy_audio_SequenceEntry* entry;

			entry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (!entry) {
				psy_audio_OrderIndex index;

				p = psy_list_remove(&track->entries, p);
				index.order = c;
				index.track = trackidx;
				psy_signal_emit(&self->signal_remove, self, 1, &index);
				removed = TRUE;
			} else {
				psy_list_next(&p);
				++c;
			}			
		}
		if (removed) {
			psy_audio_sequence_reposition_track(self, track);
		}
	}
	if (removed) {
		psy_signal_emit(&self->signal_changed, self, 0);
	}
}

uintptr_t psy_audio_sequence_order(psy_audio_Sequence* self,
	uintptr_t trackidx, psy_dsp_big_beat_t position)
{
	psy_audio_SequenceTrack* track;

	assert(self);

	if (position < 0.0) {
		return psy_INDEX_INVALID;
	}
	track = psy_audio_sequence_track_at(self, trackidx);
	if (track) {
		psy_dsp_big_beat_t curroffset;
		psy_List* p;
		uintptr_t row;
		bool found;
		
		found = FALSE;
		for (p = track->entries, row = 0, curroffset = 0.0; p != NULL;
				psy_list_next(&p), ++row) {
			psy_audio_Pattern* pattern;
			psy_audio_SequenceEntry* entry;

			entry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (entry) {
				pattern = psy_audio_patterns_at(self->patterns,
					psy_audio_sequenceentry_patternslot(entry));
				if (pattern && position < entry->offset +
					psy_audio_pattern_length(pattern)) {
					found = TRUE;
					break;
				}
			}
		}
		if (found) {
			return row;
		}
	}
	return psy_INDEX_INVALID;
}

psy_audio_SequenceEntry* psy_audio_sequence_entry(psy_audio_Sequence* self,
	psy_audio_OrderIndex index)
{
	psy_audio_SequenceTrack* track;

	assert(self);

	if (psy_audio_orderindex_valid(&index)) {
		track = psy_audio_sequence_track_at(self, index.track);
		if (track) {
			psy_List* p;

			p = psy_list_at(track->entries, index.order);
			if (p) {
				return (psy_audio_SequenceEntry*)psy_list_entry(p);
			}
		}
	}
	return NULL;
}

psy_audio_SequenceEntryNode* psy_audio_sequence_node(psy_audio_Sequence* self,
	psy_audio_OrderIndex index, psy_audio_SequenceTrack** rv)
{
	psy_audio_SequenceTrack* track;

	assert(self);

	if (psy_audio_orderindex_valid(&index)) {
		track = psy_audio_sequence_track_at(self, index.track);
		if (track) {
			psy_List* p;

			p = psy_list_at(track->entries, index.order);
			if (p) {	
				*rv = track;
				return p;
			}
		}
	}
	*rv = NULL;
	return NULL;
}


const psy_audio_SequenceEntry* psy_audio_sequence_entry_const(const
	psy_audio_Sequence* self, psy_audio_OrderIndex index)
{
	return psy_audio_sequence_entry((psy_audio_Sequence*)self, index);
}

psy_audio_Pattern* psy_audio_sequence_pattern(psy_audio_Sequence* self,
	psy_audio_OrderIndex index)
{
	psy_audio_SequenceEntry* entry;

	assert(self);

	entry = psy_audio_sequence_entry(self, index);
	if (entry) {
		return psy_audio_patterns_at(self->patterns,
			psy_audio_sequenceentry_patternslot(entry));
	}
	return NULL;
}

uintptr_t psy_audio_sequence_track_size(const psy_audio_Sequence* self,
	uintptr_t trackindex)
{
	const psy_audio_SequenceTrack* track;

	assert(self);

	track = psy_audio_sequence_track_at((psy_audio_Sequence*)self, trackindex);
	if (track) {		
		return psy_list_size(track->entries);
	}
	return 0;
}

void psy_audio_sequence_setpatternindex(psy_audio_Sequence* self,
	psy_audio_OrderIndex index, uintptr_t patidx)
{
	psy_audio_SequenceEntry* entry;

	assert(self);

	entry = psy_audio_sequence_entry(self, index);
	if (entry) {
		psy_audio_Pattern* pattern;
		psy_dsp_big_beat_t oldlength;

		pattern = psy_audio_patterns_at(self->patterns, entry->patternslot);
		if (pattern) {
			oldlength = psy_audio_pattern_length(pattern);
		} else {
			oldlength = 0.0;
		}
		pattern = psy_audio_patterns_at(self->patterns, patidx);
		if (pattern == NULL) {						
			pattern = psy_audio_patterns_insert(self->patterns, patidx,
				psy_audio_pattern_allocinit());
			// todo set correct default lines
		}						
		psy_audio_sequenceentry_setpatternslot(entry, patidx);
		if (psy_audio_pattern_length(pattern) != oldlength) {
			psy_audio_SequenceTrack* track;

			track = psy_audio_sequence_track_at(self, index.track);
			if (track) {
				psy_audio_sequence_reposition_track(self, track);
			}
		}
	}
}

uintptr_t psy_audio_sequence_patternindex(const psy_audio_Sequence* self,
	psy_audio_OrderIndex index)
{
	const psy_audio_SequenceEntry* entry;

	assert(self);

	entry = psy_audio_sequence_entry_const(self, index);
	if (entry) {
		return entry->patternslot;		
	}
	return psy_INDEX_INVALID;
}

psy_dsp_big_beat_t psy_audio_sequence_offset(const psy_audio_Sequence* self,
	psy_audio_OrderIndex index)
{
	const psy_audio_SequenceEntry* entry;

	assert(self);

	entry = psy_audio_sequence_entry_const(self, index);
	if (entry) {
		return entry->offset;
	}
	return (psy_dsp_big_beat_t)0.0;
}

void psy_audio_sequence_swaptracks(psy_audio_Sequence* self,
	uintptr_t srcid, uintptr_t dstid)
{
	psy_audio_SequenceTrackNode* src_track;	

	assert(self);

	src_track = psy_list_at(self->tracks, srcid);
	if (src_track) {
		psy_audio_SequenceTrackNode* dst_track;

		dst_track = psy_list_at(self->tracks, dstid);
		if (dst_track) {	
			uintptr_t num;
			psy_audio_SequenceTrack** arrayptr;

			num = psy_list_size(self->tracks);
			if (num != 0) {
				arrayptr = malloc(sizeof(psy_audio_SequenceTrack*) * num);
				if (arrayptr) {
					psy_List* p;
					uintptr_t i;
					psy_audio_SequenceTrack* temp;

					p = self->tracks;
					for (i = 0; p != NULL && i < num; p = p->next, ++i) {
						arrayptr[i] = p->entry;
					}
					temp = arrayptr[srcid];
					arrayptr[srcid] = arrayptr[dstid];
					arrayptr[dstid] = temp;
					psy_list_free(self->tracks);
					self->tracks = NULL;
					for (i = 0; i < num; ++i) {
						psy_list_append(&self->tracks, arrayptr[i]);
					}
					free(arrayptr);
					arrayptr = NULL;
					psy_signal_emit(&self->signal_trackswap, self, 1, srcid, dstid);
					psy_signal_emit(&self->signal_changed, self, 0);
				}				
			}
		}
	}
}

psy_audio_OrderIndex psy_audio_sequence_reorder(psy_audio_Sequence* self,
	psy_audio_OrderIndex index, psy_dsp_big_beat_t newposition)
{	
	psy_audio_SequenceTrack* track;	

	assert(self);	

	track = psy_audio_sequence_track_at(self, index.track);
	if (track) {		
		psy_audio_SequenceEntry* selected;
		
		if (selected = psy_audio_sequence_entry(self, index)) {
			psy_audio_SequenceEntry oldentry;
			psy_audio_SequenceEntry* newentry;
			psy_audio_Pattern* pattern;
			bool append;
			uintptr_t row;
			psy_dsp_big_beat_t curroffset;
			psy_List* p;

			pattern = psy_audio_patterns_at(self->patterns,
				psy_audio_sequenceentry_patternslot(selected));
			if (pattern && newposition >= selected->offset - selected->repositionoffset &&
					newposition < selected->offset + pattern->length) {				
				return index;
			}
			oldentry = *selected;
			psy_audio_sequence_remove(self, index);
			selected = NULL;
			psy_audio_sequence_reposition_track(self, track);
			append = TRUE;
			for (p = track->entries, row = 0, curroffset = 0.0; p != NULL;
					psy_list_next(&p), ++row) {
				psy_audio_Pattern* pattern;
				psy_audio_SequenceEntry* entry;

				entry = (psy_audio_SequenceEntry*)psy_list_entry(p);
				pattern = psy_audio_patterns_at(self->patterns,
					psy_audio_sequenceentry_patternslot(entry));
				curroffset = entry->offset;
				if (newposition < curroffset + pattern->length) {					
					if (newposition <= curroffset) {
						p = p->prev;
					}
					append = FALSE;										
					break;
				}
			}
			newentry = psy_audio_sequenceentry_allocinit(0,
				(psy_dsp_big_beat_t)0.0);
			if (newentry) {
				psy_audio_OrderIndex insertindex;

				insertindex = psy_audio_orderindex_make(index.track, row);
				*newentry = oldentry;
				if (!append) {
					psy_list_insert(&track->entries, p, newentry);
				} else {
					psy_list_append(&track->entries, newentry);
				}
				psy_audio_sequence_reposition_track(self, track);
				psy_signal_emit(&self->signal_insert, self, 1, &insertindex);
				psy_signal_emit(&self->signal_changed, self, 0);
				return insertindex;
			}
		}
	}
	return psy_audio_orderindex_make(psy_INDEX_INVALID, psy_INDEX_INVALID);
}

void psy_audio_sequence_resetpatterns(psy_audio_Sequence* self)
{
	psy_audio_SequenceTrackNode* t;	

	assert(self);
	
	for (t = self->tracks; t != NULL; psy_list_next(&t)) {
		psy_audio_SequenceTrack* track;
		psy_List* p;

		track = (psy_audio_SequenceTrack*)psy_list_entry(t);
		for (p = track->entries; p != NULL; psy_list_next(&p)) {
			psy_audio_SequenceEntry* entry;			

			entry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (entry) {
				// todo default lines
				psy_audio_patterns_insert(self->patterns, entry->patternslot,
					psy_audio_pattern_allocinit());
			}
		}
	}
}

void psy_audio_sequence_fillpatterns(psy_audio_Sequence* self)
{
	psy_audio_SequenceTrackNode* t;

	assert(self);

	for (t = self->tracks; t != NULL; psy_list_next(&t)) {
		psy_audio_SequenceTrack* track;
		psy_List* p;

		track = (psy_audio_SequenceTrack*)psy_list_entry(t);
		for (p = track->entries; p != NULL; psy_list_next(&p)) {
			psy_audio_SequenceEntry* entry;

			entry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (!psy_audio_patterns_at(self->patterns, entry->patternslot)) {
				psy_audio_patterns_insert(self->patterns, entry->patternslot,
					psy_audio_pattern_allocinit());
			}
		}
	}
}