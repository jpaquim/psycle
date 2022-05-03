/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sequence.h"
/* local */
#include "sequencer.h" /* calculate duration */
#include "sequencecmds.h"
/* container */
#include <properties.h>
/* platform */
#include "../../detail/portable.h"

/* psy_audio_Order */
void psy_audio_order_init(psy_audio_Order* self)
{
	self->index = psy_audio_orderindex_make_invalid();
	self->entry = NULL;
}

void psy_audio_order_dispose(psy_audio_Order* self)
{
	free(self->entry);
	self->entry = NULL;
}

void psy_audio_order_setentry(psy_audio_Order* self,
	const psy_audio_SequenceEntry* entry)
{
	free(self->entry);
	self->entry = NULL;
	self->entry = psy_audio_sequenceentry_clone(entry);
}

/* psy_audio_SequenceTrack */

static void psy_audio_sequencetrack_update_entries(psy_audio_SequenceTrack*);

void psy_audio_sequencetrack_init(psy_audio_SequenceTrack* self)
{
	assert(self);

	self->nodes = NULL;
	psy_table_init(&self->entries);
	self->name = psy_strdup("seq");
	self->height = 0.0;
}

void psy_audio_sequencetrack_dispose(psy_audio_SequenceTrack* self)
{
	assert(self);

	psy_table_dispose(&self->entries);
	psy_list_deallocate(&self->nodes,
		(psy_fp_disposefunc)psy_audio_sequenceentry_dispose);
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

void psy_audio_sequencetrack_deallocate(psy_audio_SequenceTrack* self)
{
	psy_audio_sequencetrack_dispose(self);
	free(self);
	self = NULL;
}

psy_audio_SequenceTrack* psy_audio_sequencetrack_clone(
	psy_audio_SequenceTrack* src)
{
	psy_audio_SequenceTrack* rv;

	assert(src);

	rv = psy_audio_sequencetrack_allocinit();
	if (rv) {
		psy_List* p;
		uintptr_t i;
				
		for (p = src->nodes, i = 0; p != NULL; psy_list_next(&p), ++i) {
			psy_audio_SequenceEntry* seqentry;
			psy_List* q;
			
			seqentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (seqentry) {
				q = psy_list_append(&rv->nodes, psy_audio_sequenceentry_clone(
					seqentry));
			} else {
				q = psy_list_append(&rv->nodes, NULL);
			}
			psy_table_insert(&rv->entries, i, q);
		}
		psy_strreset(&rv->name, src->name);
	}
	return rv;
}

psy_dsp_big_beat_t psy_audio_sequencetrack_duration(const psy_audio_SequenceTrack* self,
	const psy_audio_Patterns* patterns)
{
	psy_dsp_big_beat_t rv;
	const psy_List* p;

	assert(self);

	rv = 0.0;
	p = self->nodes;
	if (p) {		
		const psy_audio_SequenceEntry* seqentry;

		p = p->tail;
		seqentry = (const psy_audio_SequenceEntry*)p->entry;
		if (seqentry) {
			psy_dsp_big_beat_t length;

			length = psy_audio_sequenceentry_length(seqentry);
			if (rv < psy_audio_sequenceentry_offset(seqentry) + length) {
				rv = psy_audio_sequenceentry_offset(seqentry) + length;
			}
		}
	}
	return rv;
}

void psy_audio_sequencetrack_setname(psy_audio_SequenceTrack* self,
	const char* name)
{
	assert(self);

	self->name = psy_strreset(&self->name, name);	
}

void psy_audio_sequencetrack_insert(psy_audio_SequenceTrack* self,
	uintptr_t order, psy_audio_Patterns* patterns, uintptr_t patidx)
{	
	psy_List* p;
	psy_audio_SequencePatternEntry* entry;

	assert(self);

	p = psy_list_at(self->nodes, order);
	if (!p) {
		p = psy_list_last(self->nodes);
	}
	entry = psy_audio_sequencepatternentry_allocinit(patidx,
		(psy_dsp_big_beat_t)0.0);
	entry->patterns = patterns;
	p = psy_list_insert(&self->nodes, p, entry);
	psy_table_insert(&self->entries, order, p);
}

uintptr_t psy_audio_sequencetrack_remove(psy_audio_SequenceTrack* self,
	uintptr_t order)
{	
	psy_List* p;

	assert(self);

	p = psy_list_at(self->nodes, order);
	if (p) {
		psy_audio_SequenceEntry* entry;
		uintptr_t patidx;

		entry = (psy_audio_SequenceEntry*)p->entry;
		if (entry) {
			patidx = psy_INDEX_INVALID;
			if (entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
				patidx = ((psy_audio_SequencePatternEntry*)entry)->patternslot;
			}
			psy_list_remove(&self->nodes, p);
			psy_audio_sequenceentry_dispose(entry);
			free(entry);
			entry = NULL;
			psy_table_remove(&self->entries, order);
			return patidx;
		}		
	}
	return psy_INDEX_INVALID;
}

void psy_audio_sequencetrack_update_entries(psy_audio_SequenceTrack* self)
{
	psy_audio_SequenceEntryNode* t;
	uintptr_t i;

	assert(self);

	psy_table_clear(&self->entries);
	t = self->nodes;
	i = 0;
	while (t != NULL) {		
		psy_table_insert(&self->entries, i, t);
		++i;
		t = t->next;
	}
}

psy_audio_SequenceEntry* psy_audio_sequencetrack_entry(psy_audio_SequenceTrack* self,
	uintptr_t row)
{	
	psy_List* p;
	
	assert(self);
		
	p = psy_table_at(&self->entries, row);
	if (p) {
		return (psy_audio_SequenceEntry*)p->entry;
	}
	return NULL;
}

psy_audio_SequenceEntryNode* psy_audio_sequencetrack_node_at_offset(
	psy_audio_SequenceTrack* self, psy_dsp_big_beat_t offset)
{	
	uintptr_t mid;
	uintptr_t l;
	uintptr_t r;	

	if (psy_audio_sequencetrack_size(self) == 0) {
		return NULL;
	}
	l = 0;
	r = psy_audio_sequencetrack_size(self) - 1;	
	while (r >= l) {
		psy_audio_SequenceEntry* entry;

		mid = (l + r) / 2;
		entry = psy_audio_sequencetrack_entry(self, mid);
		if (!entry) {
			mid = psy_INDEX_INVALID;
			break;
		}		
		if (offset >= psy_audio_sequenceentry_offset(entry) &&
			offset < psy_audio_sequenceentry_right_offset(entry)) {
			break;
		} else {
			if (offset > psy_audio_sequenceentry_offset(entry)) {
				l = mid + 1;
			} else {
				r = mid - 1;
			}
		}
	}
	if (mid > 0) {
		psy_audio_SequenceEntry* entry;

		entry = psy_audio_sequencetrack_entry(self, mid);
		if (entry) {
			psy_dsp_big_beat_t offset;

			offset = psy_audio_sequenceentry_offset(entry);
			l = 0;			
			r = mid - 1;
			do {
				psy_audio_SequenceEntry* entry;

				entry = psy_audio_sequencetrack_entry(self, r);
				if (entry && psy_audio_sequenceentry_offset(entry) < offset) {
					break;
				}
				mid = r;
				if (r > 0) {
					--r;
				}
			} while (r >= 0);
		}
	}
	return psy_table_at(&self->entries, mid);	
}

uintptr_t psy_audio_sequencetrack_size(const psy_audio_SequenceTrack* self)
{
	return psy_table_size(&self->entries);
}

/* psy_audio_SequencePosition */
void psy_audio_sequenceposition_init(psy_audio_SequencePosition* self)
{
	assert(self);

	self->track_index = psy_INDEX_INVALID;
	psy_audio_sequencetrackiterator_init(&self->trackposition);	
}

void psy_audio_sequenceposition_dispose(psy_audio_SequencePosition* self)
{
	psy_audio_sequencetrackiterator_dispose(&self->trackposition);
}

psy_audio_SequenceEntry* psy_audio_sequenceposition_entry(
	psy_audio_SequencePosition* self)
{
	assert(self);

	if (self->trackposition.sequencentrynode) {
		return (psy_audio_SequenceEntry*)
			self->trackposition.sequencentrynode->entry;
	}
	return NULL;
}

/* Sequence */
/* prototypes */
static void psy_audio_sequence_initsignals(psy_audio_Sequence*);
static void psy_audio_sequence_initglobaltrack(psy_audio_Sequence*);
static void psy_audio_sequence_disposesignals(psy_audio_Sequence*);
static void sequence_onpatternlengthchanged(psy_audio_Sequence*,
	psy_audio_Pattern* sender);
static void sequence_onpatternnamechanged(psy_audio_Sequence*,
	psy_audio_Pattern* sender);
static void sequence_makeiterator(psy_audio_Sequence*, psy_List* entries,
	psy_audio_SequenceTrackIterator* rv);

/* implementation */
void psy_audio_sequence_init(psy_audio_Sequence* self,
	psy_audio_Patterns* patterns, psy_audio_Samples* samples)
{
	assert(self);

	psy_audio_sequence_initsignals(self);	
	psy_table_init(&self->tracks);
	self->patterns = patterns;
	self->samples = samples;
	self->preventreposition = FALSE;
	self->sequencerduration = NULL;
	psy_audio_sequence_initsignals(self);	
	psy_audio_sequence_initglobaltrack(self);
	psy_audio_trackstate_init(&self->trackstate);
	/* init editposition */
	psy_audio_sequencecursor_init(&self->cursor);
	self->lastcursor = self->cursor;
	psy_audio_sequenceselection_init(&self->sequenceselection);
	psy_audio_sequenceselection_select(&self->sequenceselection,
		psy_audio_orderindex_make(0, 0));	
}

void psy_audio_sequence_initglobaltrack(psy_audio_Sequence* self)
{	
	psy_audio_SequencePatternEntry* entry;

	psy_audio_sequencetrack_init(&self->globaltrack);	
	entry = psy_audio_sequencepatternentry_allocinit(psy_audio_GLOBALPATTERN,
		(psy_dsp_big_beat_t)0.0);
	entry->patterns = self->patterns;
	psy_list_append(&self->globaltrack.nodes, entry);
	psy_audio_sequence_reposition_track(self, &self->globaltrack);
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
	psy_signal_init(&self->signal_cursorchanged);
}

void psy_audio_sequence_dispose(psy_audio_Sequence* self)
{
	psy_table_disposeall(&self->tracks, (psy_fp_disposefunc)
		psy_audio_sequencetrack_dispose);
	psy_audio_sequencetrack_dispose(&self->globaltrack);	
	psy_audio_trackstate_dispose(&self->trackstate);	
	psy_audio_sequence_disposesignals(self);	
	if (self->sequencerduration) {
		psy_audio_sequencer_dispose(self->sequencerduration);
		free(self->sequencerduration);
		self->sequencerduration = NULL;
	}
	psy_audio_sequenceselection_dispose(&self->sequenceselection);	
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
	psy_signal_dispose(&self->signal_cursorchanged);
}

void psy_audio_sequence_copy(psy_audio_Sequence* self, psy_audio_Sequence* other)
{	
	uintptr_t t;

	psy_audio_sequence_clear(self);
	for (t = 0; t < psy_audio_sequence_num_tracks(self); ++t) {
		psy_audio_SequenceTrack* src_track;

		src_track = psy_audio_sequence_track_at(self, t);
		if (src_track) {
			psy_audio_sequence_append_track(self, psy_audio_sequencetrack_clone(src_track));
		}
	}
	psy_audio_trackstate_copy(&self->trackstate, &other->trackstate);	
	self->patterns = other->patterns;	
}

void psy_audio_sequence_clear(psy_audio_Sequence* self)
{
	psy_table_disposeall(&self->tracks, (psy_fp_disposefunc)
		psy_audio_sequencetrack_dispose);
	psy_table_init(&self->tracks);	
	psy_audio_trackstate_dispose(&self->trackstate);
	psy_audio_trackstate_init(&self->trackstate);
	psy_signal_emit(&self->signal_clear, self, 0);
}

void psy_audio_sequence_reposition_track(psy_audio_Sequence* self,
	psy_audio_SequenceTrack* track)
{
	if (!self->preventreposition) {
		psy_dsp_big_beat_t curroffset;	
		psy_List* p;
		uintptr_t row;
		uintptr_t trackidx;
			
		for (p = track->nodes, row = 0, curroffset = 0.0; p != NULL;
				psy_list_next(&p), ++row) {			
			psy_audio_SequenceEntry* seqentry;
		
			seqentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (seqentry) {
				seqentry->row = row;
				seqentry->offset = curroffset +
					seqentry->repositionoffset;
				curroffset = psy_audio_sequenceentry_offset(seqentry) +
					psy_audio_sequenceentry_length(seqentry);
			}
		}
		psy_audio_sequencetrack_update_entries(track);
		for (trackidx = 0; trackidx < psy_audio_sequence_num_tracks(self); ++trackidx) {
			if (psy_audio_sequence_track_at(self, trackidx) == track) {
				break;
			}			
		}		
		psy_signal_emit(&self->signal_trackreposition, self, 1, trackidx);
	}
}

void psy_audio_reposition(psy_audio_Sequence* self)
{
	uintptr_t t;

	for (t = 0; t < psy_audio_sequence_num_tracks(self); ++t) {
		psy_audio_SequenceTrack* track;

		track = psy_audio_sequence_track_at(self, t);
		if (track) {
			psy_audio_sequence_reposition_track(self, track);
		}
	}
}

void psy_audio_sequence_at(psy_audio_Sequence* self, uintptr_t trackindex,
	uintptr_t position, psy_audio_SequencePosition* rv)
{	
	psy_List* ptr;	
	uintptr_t c = 0;	
	psy_audio_SequenceTrack* track;
	
	assert(rv);

	rv->trackposition.patterns = self->patterns;
	track = psy_audio_sequence_track_at(self, trackindex);	
	rv->track_index = trackindex;
	if (rv->track_index != psy_INDEX_INVALID) {
		psy_audio_SequenceTrack* track;

		track = psy_audio_sequence_track_at(self, rv->track_index);
		if (!track) {
			return;
		}
		ptr = track->nodes;
		c = 0;
		while (ptr) {
			if (c == position) {			
				psy_audio_sequence_make_position(self, rv->track_index, ptr, rv);
				break;
			}
			++c;
			ptr = ptr->next;
		}
	}	
}

void psy_audio_sequence_begin(psy_audio_Sequence* self,
	uintptr_t track_idx,
	psy_dsp_big_beat_t position,
	psy_audio_SequenceTrackIterator* rv)
{			
	psy_audio_SequenceEntry* entry;	
	psy_audio_SequenceTrack* track;

	rv->patterns = self->patterns;
	if (track_idx == psy_audio_GLOBALTRACK) {		
		track = &self->globaltrack;
	} else {
		track = psy_audio_sequence_track_at(self, track_idx);
	}
	if (!track) {
		rv->sequencentrynode = NULL;
		rv->pattern = NULL;
		rv->patternnode = NULL;		
	} else {
		rv->sequencentrynode = psy_audio_sequencetrack_node_at_offset(track, position);
		if (rv->sequencentrynode) {
			entry = (psy_audio_SequenceEntry*)rv->sequencentrynode->entry;
			if (entry && entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
				psy_audio_SequencePatternEntry* seqpatternentry;

				seqpatternentry = (psy_audio_SequencePatternEntry*)rv->sequencentrynode->entry;
				psy_audio_sequencetrackiterator_set_pattern(rv,
					psy_audio_patterns_at(self->patterns, seqpatternentry->patternslot));
				if (rv->pattern) {
					rv->patternnode = psy_audio_pattern_greaterequal(rv->pattern,
						position - entry->offset);
				} else {
					rv->patternnode = NULL;
				}
			} else {
				rv->pattern = NULL;
				rv->patternnode = NULL;
			}
		} else {
			rv->pattern = NULL;
			rv->patternnode = NULL;
		}
	}
}

void psy_audio_sequencetrackiterator_init(psy_audio_SequenceTrackIterator* self)
{	
	self->patterns = NULL;
	self->sequencentrynode = NULL;
	self->patternnode = NULL;	
	self->pattern = NULL;
}

void psy_audio_sequencetrackiterator_dispose(psy_audio_SequenceTrackIterator* self)
{
	if (self->pattern) {
		psy_audio_pattern_remove_seqiterator(self->pattern, self);
		self->pattern = NULL;
	}	
}

psy_audio_SequenceTrackIterator* psy_audio_sequencetrackiterator_alloc(void)
{
	return (psy_audio_SequenceTrackIterator*)malloc(sizeof(
		psy_audio_SequenceTrackIterator));
}

psy_audio_SequenceTrackIterator* psy_audio_sequencetrackiterator_allocinit(void)
{
	psy_audio_SequenceTrackIterator* rv;

	rv = psy_audio_sequencetrackiterator_alloc();
	if (rv) {
		psy_audio_sequencetrackiterator_init(rv);
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

				do {
					self->sequencentrynode = self->sequencentrynode->next;
					entry = (psy_audio_SequenceEntry*)self->sequencentrynode->entry;
				} while (entry->type != psy_audio_SEQUENCEENTRY_PATTERN);
				if (entry) {
					psy_audio_SequencePatternEntry* seqpatternentry;

					seqpatternentry = (psy_audio_SequencePatternEntry*)entry;					
					pattern = psy_audio_patterns_at(self->patterns,
						seqpatternentry->patternslot);
					psy_audio_sequencetrackiterator_set_pattern(self, pattern);					
				} else {
					psy_audio_sequencetrackiterator_set_pattern(self, NULL);
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
			psy_audio_SequencePatternEntry* entry;			

			entry = (psy_audio_SequencePatternEntry*)self->sequencentrynode->entry;
			if (entry && entry->entry.type == psy_audio_SEQUENCEENTRY_PATTERN) {
				psy_audio_sequencetrackiterator_set_pattern(self,					
					psy_audio_patterns_at(self->patterns, entry->patternslot));				
			} else {
				psy_audio_sequencetrackiterator_set_pattern(self, NULL);				
			}
		} else {
			psy_audio_sequencetrackiterator_set_pattern(self, NULL);
		}
	}
}

void psy_audio_sequencetrackiterator_dec_entry(psy_audio_SequenceTrackIterator* self)
{	
	if (self->sequencentrynode) {
		self->sequencentrynode = self->sequencentrynode->prev;
		if (self->sequencentrynode) {
			psy_audio_SequencePatternEntry* entry;

			entry = (psy_audio_SequencePatternEntry*) self->sequencentrynode->entry;
			if (entry && entry->entry.type == psy_audio_SEQUENCEENTRY_PATTERN) {
				psy_audio_sequencetrackiterator_set_pattern(self,
					psy_audio_patterns_at(self->patterns, entry->patternslot));
			} else {
				psy_audio_sequencetrackiterator_set_pattern(self, NULL);				
			}
		} else {
			psy_audio_sequencetrackiterator_set_pattern(self, NULL);
		}
	}
}

bool psy_audio_sequencetrackiterator_has_next_entry(
	const psy_audio_SequenceTrackIterator* self)
{
	return ((self->sequencentrynode) && (self->sequencentrynode->next));
}

void psy_audio_sequencetrackiterator_set_pattern(
	psy_audio_SequenceTrackIterator* self,
	psy_audio_Pattern* pattern)
{
	if (self->pattern && self->pattern != pattern) {
		psy_audio_pattern_remove_seqiterator(self->pattern, self);		
	}	
	self->pattern = pattern;
	if (self->pattern) {		
		psy_audio_pattern_add_seqiterator(self->pattern, self);		
		self->patternnode = self->pattern->events;
	} else {
		self->patternnode = NULL;
	}
}

void sequence_makeiterator(psy_audio_Sequence* self, psy_List* entries,
	psy_audio_SequenceTrackIterator* rv)
{	
	rv->patterns = self->patterns;
	rv->sequencentrynode = entries;
	if (entries) {
		psy_audio_SequencePatternEntry* entry = (psy_audio_SequencePatternEntry*)entries->entry;
		if (entry && entry->entry.type == psy_audio_SEQUENCEENTRY_PATTERN) {
			psy_audio_sequencetrackiterator_set_pattern(rv,
				psy_audio_patterns_at(self->patterns, entry->patternslot));			
		} else {
			psy_audio_sequencetrackiterator_set_pattern(rv, NULL);			
		}
	}	
}

void psy_audio_sequence_make_position(psy_audio_Sequence* self,
	uintptr_t track_idx, psy_List* entries,
	psy_audio_SequencePosition* rv)
{
	sequence_makeiterator(self, entries, &rv->trackposition);
	rv->track_index = track_idx;
}

void psy_audio_sequence_append_track(psy_audio_Sequence* self,
	psy_audio_SequenceTrack* track)
{	
	assert(self);

	psy_table_insert(&self->tracks, psy_table_size(&self->tracks), track);	
	psy_signal_emit(&self->signal_trackinsert, self, 1,
		psy_table_size(&self->tracks) - 1);
	psy_signal_emit(&self->signal_changed, self, 0);
}

uintptr_t psy_audio_sequence_set_track(psy_audio_Sequence* self,
	psy_audio_SequenceTrack* track, uintptr_t index)
{
	assert(self);
		
	psy_table_insert(&self->tracks, index, track);
	psy_signal_emit(&self->signal_trackinsert, self, 1, index);
	psy_signal_emit(&self->signal_changed, self, 0);
	return (index + 1);
}

void psy_audio_sequence_remove_track(psy_audio_Sequence* self,
	uintptr_t track_idx)
{
	psy_audio_SequenceTrack* track;
	uintptr_t size;
	uintptr_t t;

	assert(self);	
	
	size = psy_table_size(&self->tracks);
	if (size == 0) {
		return;
	}
	track = psy_audio_sequence_track_at(self, track_idx);
	for (t = track_idx; t < size; ++t) {
		psy_audio_SequenceTrack* curr;

		curr = psy_audio_sequence_track_at(self, t + 1);
		psy_table_insert(&self->tracks, t, curr);
	}	
	psy_table_remove(&self->tracks, size - 1);	
	if (track) {
		psy_audio_sequencetrack_dispose(track);
	}
	psy_signal_emit(&self->signal_trackremove, self, 1, track_idx);
	psy_signal_emit(&self->signal_changed, self, 0);
}

psy_audio_SequenceTrack* psy_audio_sequence_track_at(psy_audio_Sequence* self,
	uintptr_t index)
{
	assert(self);
	
	return (psy_audio_SequenceTrack*)psy_table_at(&self->tracks, index);
}

const psy_audio_SequenceTrack* psy_audio_sequence_track_at_const(const
	psy_audio_Sequence* self, uintptr_t index)
{
	assert(self);

	return psy_audio_sequence_track_at((psy_audio_Sequence*)self, index);
}

uintptr_t psy_audio_sequence_num_tracks(const psy_audio_Sequence* self)
{
	return psy_table_size(&self->tracks);
}

uintptr_t psy_audio_sequence_width(const psy_audio_Sequence* self)
{
	return psy_table_size(&self->tracks);	
}

bool psy_audio_sequence_patternused(psy_audio_Sequence* self, uintptr_t patternslot)
{
	int rv = FALSE;
	uintptr_t t;

	assert(self);
	
	for (t = 0; t < psy_audio_sequence_width(self); ++t) {
		psy_audio_SequenceTrack* track;
		psy_audio_SequenceEntryNode* p;

		track = psy_audio_sequence_track_at(self, t);
		if (!track) {
			continue;
		}
		p = track->nodes;
		while (p) {
			psy_audio_SequencePatternEntry* sequenceentry;

			sequenceentry = (psy_audio_SequencePatternEntry*)psy_list_entry(p);
			if (sequenceentry) {
				if (psy_audio_sequencepatternentry_patternslot(sequenceentry) ==
					patternslot) {
					rv = TRUE;
					break;
				}
			}
			psy_list_next(&p);
		}		
	}
	return rv;
}

psy_audio_OrderIndex psy_audio_sequence_patternfirstused(psy_audio_Sequence* self,
	uintptr_t patternslot)
{
	psy_audio_OrderIndex rv;
	uintptr_t t;	

	assert(self);
	rv.track = psy_INDEX_INVALID;
	rv.order = psy_INDEX_INVALID;
			
	for (t = 0; t < psy_audio_sequence_width(self); ++t) {
		psy_audio_SequenceTrack* track;
		psy_audio_SequenceEntryNode* p;
		uintptr_t currorderidx;

		track = psy_audio_sequence_track_at(self, t);
		if (!track) {
			continue;
		}
		currorderidx = 0;
		p = track->nodes;
		while (p) {
			psy_audio_SequenceEntry* entry;

			entry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (entry && entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
				psy_audio_SequencePatternEntry* seqpatternentry;

				seqpatternentry = (psy_audio_SequencePatternEntry*)entry;
				if (psy_audio_sequencepatternentry_patternslot(seqpatternentry)
						== patternslot) {
					psy_audio_Pattern* pattern;

					pattern = psy_audio_patterns_at(self->patterns, patternslot);
					rv.track = t;
					rv.order = currorderidx;
					break;
				}
			}
			psy_list_next(&p);
			++currorderidx;
		}				
	}
	return rv;
}

psy_dsp_big_beat_t psy_audio_sequence_duration(const psy_audio_Sequence* self)
{	
	uintptr_t t;
	psy_dsp_big_beat_t rv;

	assert(self);

	rv = 0.0;	
	for (t = 0; t < psy_audio_sequence_width(self); ++t) {
		const psy_audio_SequenceTrack* track;          
		psy_dsp_big_beat_t trackduration;

		track = psy_audio_sequence_track_at_const(self, t);
		if (!track) {
			continue;
		}
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
	uintptr_t t;
	
	assert(self);

	for (t = 0; t < psy_audio_sequence_width(self); ++t) {
		psy_audio_SequenceTrack* track;
		uintptr_t tracksize;

		track = psy_audio_sequence_track_at(self, t);
		if (!track) {
			continue;
		}
		tracksize = psy_list_size(track->nodes);
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
	uintptr_t t;
	
	for (t = 0; t < psy_audio_sequence_width(self); ++t) {
		psy_audio_SequenceTrack* track;
		psy_List* p;

		track = psy_audio_sequence_track_at(self, t);
		if (!track) {
			continue;
		}
		for (p = track->nodes; p != NULL; psy_list_next(&p)) {
			psy_audio_SequencePatternEntry* entry;

			entry = (psy_audio_SequencePatternEntry*)p->entry;
			if (entry) {
				entry->entry.selplay = FALSE;
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
	psy_audio_sequencer_stop_loop(self->sequencerduration);
	psy_audio_sequencer_start(self->sequencerduration);
	self->sequencerduration->calcduration = TRUE;
}

psy_dsp_big_seconds_t psy_audio_sequence_endcalcdurationinmsresult(psy_audio_Sequence* self)
{	
	if (self->sequencerduration) {
		psy_dsp_big_seconds_t rv;

		rv = psy_audio_sequencer_curr_play_time(self->sequencerduration);
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
	numsamplex = 8192; /* psy_audio_MAX_STREAM_SIZE; */
	maxamount = numsamplex;		
	do {
		amount = maxamount;
		if (amount > numsamplex) {
			amount = numsamplex;
		}
		if (self->sequencerduration->linetickcount <=
			psy_audio_sequencer_frame_to_offset(self->sequencerduration, amount)) {
			if (self->sequencerduration->linetickcount > 0) {
				uintptr_t pre;

				pre = psy_audio_sequencer_frames(self->sequencerduration,
					self->sequencerduration->linetickcount);
				if (pre) {
					pre--;
					if (pre) {
						psy_audio_sequencer_frame_tick(self->sequencerduration, pre);
						numsamplex -= pre;
						amount -= pre;
						self->sequencerduration->linetickcount -=
							psy_audio_sequencer_frame_to_offset(
								self->sequencerduration, pre);
					}
				}
			}					
			psy_audio_sequencer_onnewline(self->sequencerduration);
		}			
		if (amount > 0) {
			psy_audio_sequencer_frame_tick(self->sequencerduration, amount);
			numsamplex -= amount;
			self->sequencerduration->linetickcount -=
				psy_audio_sequencer_frame_to_offset(self->sequencerduration,
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

/* TrackState */
void psy_audio_sequence_activatesolotrack(psy_audio_Sequence* self,
	uintptr_t track)
{
	assert(self);

	psy_audio_trackstate_activatesolotrack(&self->trackstate, track);
	psy_signal_emit(&self->signal_solochanged, self, 1, track);
}

void psy_audio_sequence_deactivatesolotrack(psy_audio_Sequence* self)
{
	uintptr_t soloedtrack;

	assert(self);	
	
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

int psy_audio_sequence_istrackmuted(const psy_audio_Sequence* self,
	uintptr_t trackindex)
{
	assert(self);

	return psy_audio_trackstate_istrackmuted(&self->trackstate, trackindex);
}

bool psy_audio_sequence_istracksoloed(const psy_audio_Sequence* self, uintptr_t track)
{
	assert(self);

	return psy_audio_trackstate_istracksoloed(&self->trackstate, track);
}

double psy_audio_sequence_trackheight(const psy_audio_Sequence* self,
	uintptr_t trackindex)
{
	const psy_audio_SequenceTrack* track;

	assert(self);

	track = psy_audio_sequence_track_at_const(self, trackindex);
	if (track) {
		return track->height;
	}
	return 0.0;
}

void psy_audio_sequence_settrackheight(psy_audio_Sequence* self,
	uintptr_t trackindex, double height)
{
	psy_audio_SequenceTrack* track;

	assert(self);

	track = psy_audio_sequence_track_at(self, trackindex);
	if (track) {
		track->height = height;
	}	
}

/* EditPosition */

void psy_audio_sequence_set_cursor(psy_audio_Sequence* self,
	psy_audio_SequenceCursor cursor)
{
	// if (!psy_audio_sequencecursor_equal(&cursor, &self->cursor)) {
		psy_audio_SequenceEntry* entry;

		self->cursor = cursor;
		if (!psy_audio_orderindex_equal(&self->lastcursor.orderindex,
				self->cursor.orderindex)) {
			entry = psy_audio_sequence_entry(self, cursor.orderindex);
			if (entry && entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
				psy_audio_SequencePatternEntry* patternentry;

				patternentry = (psy_audio_SequencePatternEntry*)entry;
				self->cursor.seqoffset = psy_audio_sequenceentry_offset(entry);
				self->cursor.patternid = patternentry->patternslot;
			} else {
				self->cursor.seqoffset = 0.0;
				self->cursor.patternid = psy_INDEX_INVALID;
			}
			psy_audio_sequenceselection_deselect_all(&self->sequenceselection);			
			psy_audio_sequenceselection_select_first(&self->sequenceselection,
				self->cursor.orderindex);
		}
		psy_audio_sequencecursor_updatecache(&self->cursor);
		psy_signal_emit(&self->signal_cursorchanged, self, 0);
		self->lastcursor = self->cursor;
	// }
}

double psy_audio_sequence_seqoffset(const psy_audio_Sequence* self,
	psy_audio_OrderIndex index)
{
	const psy_audio_SequenceEntry* entry;
	
	entry = psy_audio_sequence_entry_const(self, index);
	if (entry) {
		return psy_audio_sequenceentry_offset(entry);
	}
	return 0.0;
}

void psy_audio_sequence_dec_seqpos(psy_audio_Sequence* self)
{
	if (psy_audio_sequenceselection_first(&self->sequenceselection).order
			> 0) {
		psy_audio_SequenceCursor cursor;

		cursor = self->cursor;
		cursor.orderindex = psy_audio_orderindex_make(
			psy_audio_sequenceselection_first(&self->sequenceselection).track,
			psy_audio_sequenceselection_first(&self->sequenceselection).order - 1);
		psy_audio_sequence_set_cursor(self, cursor);
	}
}

void psy_audio_sequence_inc_seqpos(psy_audio_Sequence* self)
{
	if (psy_audio_sequenceselection_first(&self->sequenceselection).order + 1 <
			psy_audio_sequence_track_size(self,
				psy_audio_sequenceselection_first(
					&self->sequenceselection).track)) {
		psy_audio_SequenceCursor cursor;

		cursor = self->cursor;
		cursor.orderindex = psy_audio_orderindex_make(
			psy_audio_sequenceselection_first(&self->sequenceselection).track,
			psy_audio_sequenceselection_first(&self->sequenceselection).order + 1);
		psy_audio_sequence_set_cursor(self, cursor);
	}
}


/* SequencePaste */
void psy_audio_sequencepaste_init(psy_audio_SequencePaste* self)
{
	self->nodes = NULL;	
}

void psy_audio_sequencepaste_dispose(psy_audio_SequencePaste* self)
{
	psy_list_deallocate(&self->nodes, (psy_fp_disposefunc)
		psy_audio_order_dispose);
}

void psy_audio_sequencepaste_clear(psy_audio_SequencePaste* self)
{
	psy_list_deallocate(&self->nodes, (psy_fp_disposefunc)
		psy_audio_order_dispose);
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
			psy_audio_order_init(order);
			if (order) {				
				order->index = *index;
				if (order->index.order < minindex.order) {
					minindex.order = order->index.order;
				}
				if (order->index.track < minindex.track) {
					minindex.track = order->index.track;
				}
				psy_audio_order_setentry(order, entry);				
				psy_list_append(&self->nodes, order);
			}
		}
	}
	for (p = self->nodes; p != NULL; psy_list_next(&p)) {
		psy_audio_Order* order;

		order = (psy_audio_Order*)psy_list_entry(p);
		if (psy_audio_orderindex_valid(&order->index)) {
			order->index.order -= minindex.order;
			order->index.track -= minindex.track;
		}
	}
}

/* Sequence */
void psy_audio_sequence_insert(psy_audio_Sequence* self,
	psy_audio_OrderIndex index, uintptr_t patidx)
{
	psy_audio_SequenceTrack* track;

	assert(self);

	track = psy_audio_sequence_track_at(self, index.track);
	if (track) {
		psy_audio_sequencetrack_insert(track, index.order, self->patterns, patidx);		
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

void psy_audio_sequence_insert_sample(psy_audio_Sequence* self,
	psy_audio_OrderIndex index, psy_audio_SampleIndex sampleindex)
{
	psy_audio_SequenceTrack* track;

	assert(self);

	track = psy_audio_sequence_track_at(self, index.track);
	if (track) {
		psy_List* p;
		psy_audio_SequenceSampleEntry* entry;

		p = psy_list_at(track->nodes, index.order);
		if (!p) {
			p = psy_list_last(track->nodes);
		}
		entry = psy_audio_sequencesampleentry_allocinit(
			(psy_dsp_big_beat_t)0.0, sampleindex);
		entry->samples = self->samples;
		psy_list_insert(&track->nodes, p, entry);
		psy_audio_sequence_reposition_track(self, track);
		psy_signal_emit(&self->signal_insert, self, 1, &index);
		psy_signal_emit(&self->signal_changed, self, 0);
	}
}


void psy_audio_sequence_insert_marker(psy_audio_Sequence* self,
	psy_audio_OrderIndex index, const char* text)
{
	psy_audio_SequenceTrack* track;

	assert(self);

	track = psy_audio_sequence_track_at(self, index.track);
	if (track) {
		psy_List* p;
		psy_audio_SequenceMarkerEntry* entry;

		p = psy_list_at(track->nodes, index.order);
		if (!p) {
			p = psy_list_last(track->nodes);
		}
		entry = psy_audio_sequencemarkerentry_allocinit(
			(psy_dsp_big_beat_t)0.0, text);		
		psy_list_insert(&track->nodes, p, entry);
		psy_audio_sequence_reposition_track(self, track);		
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
		uintptr_t patidx;

		patidx = psy_audio_sequencetrack_remove(track, index.order);		
		psy_audio_sequence_reposition_track(self, track);
		if (self->patterns && patidx != psy_INDEX_INVALID &&
				!psy_audio_sequence_patternused(self, patidx)) {
			psy_audio_Pattern* pattern;

			pattern = psy_audio_patterns_at(self->patterns, patidx);
			if (pattern) {
				psy_signal_disconnect_context(&pattern->signal_lengthchanged, self);
				psy_signal_disconnect_context(&pattern->signal_namechanged, self);				
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
	uintptr_t t;
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
			psy_audio_sequenceentry_dispose(currnode->entry);			
			free(currnode->entry);
			currnode->entry = NULL;
		}		
	}
	trackidx = 0;
	for (t = 0; t < psy_audio_sequence_width(self); ++t) {
		psy_audio_SequenceTrack* track;
		psy_List* p;

		track = psy_audio_sequence_track_at(self, t);
		if (!track) {
			continue;
		}	
		c = 0;		
		p = track->nodes;
		while (p != NULL) {
			psy_audio_SequenceEntry* entry;

			entry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (!entry) {
				psy_audio_OrderIndex index;

				p = psy_list_remove(&track->nodes, p);
				index = psy_audio_orderindex_make(t, c);				
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

uintptr_t psy_audio_sequence_order(const psy_audio_Sequence* self,
	uintptr_t trackidx, psy_dsp_big_beat_t position)
{
	const psy_audio_SequenceTrack* track;

	assert(self);

	if (position < 0.0) {
		return psy_INDEX_INVALID;
	}
	track = psy_audio_sequence_track_at_const(self, trackidx);
	if (track) {
		psy_dsp_big_beat_t curroffset;
		const psy_List* p;
		uintptr_t row;
		bool found;
		
		found = FALSE;
		for (p = track->nodes, row = 0, curroffset = 0.0; p != NULL;
				p = p->next, ++row) {			
			const psy_audio_SequenceEntry* seqentry;

			seqentry = (const psy_audio_SequenceEntry*)psy_list_entry_const(p);
			if (seqentry) {				
				if (position < psy_audio_sequenceentry_right_offset(seqentry)) {
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
			return psy_audio_sequencetrack_entry(track, index.order);			
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

			p = psy_list_at(track->nodes, index.order);
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
	if (entry && entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
		return psy_audio_patterns_at(self->patterns,
			psy_audio_sequencepatternentry_patternslot(
				(const psy_audio_SequencePatternEntry*)entry));
	}
	return NULL;
}

const psy_audio_Pattern* psy_audio_sequence_pattern_const(
	const psy_audio_Sequence* self, psy_audio_OrderIndex orderindex)
{
	return psy_audio_sequence_pattern((psy_audio_Sequence*)self, orderindex);
}

uintptr_t psy_audio_sequence_track_size(const psy_audio_Sequence* self,
	uintptr_t trackindex)
{
	const psy_audio_SequenceTrack* track;

	assert(self);

	track = psy_audio_sequence_track_at((psy_audio_Sequence*)self, trackindex);
	if (track) {		
		return psy_list_size(track->nodes);
	}
	return 0;
}

void psy_audio_sequence_setpatternindex(psy_audio_Sequence* self,
	psy_audio_OrderIndex index, uintptr_t patidx)
{
	psy_audio_SequenceEntry* entry;

	assert(self);

	entry = psy_audio_sequence_entry(self, index);
	if (entry && entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
		psy_audio_Pattern* pattern;
		psy_dsp_big_beat_t oldlength;
		psy_audio_SequenceTrack* track;
		
		oldlength = psy_audio_sequenceentry_length(entry);
		pattern = psy_audio_patterns_at(self->patterns, patidx);
		if (pattern == NULL) {						
			pattern = psy_audio_patterns_insert(self->patterns, patidx,
				psy_audio_pattern_allocinit());
			/* todo set correct default lines */
		}						
		psy_audio_sequencepatternentry_setpatternslot(
			(psy_audio_SequencePatternEntry*)entry, patidx);		
		track = psy_audio_sequence_track_at(self, index.track);
		if (track) {
			psy_audio_sequence_reposition_track(self, track);		
		}
	}
}

uintptr_t psy_audio_sequence_patternindex(const psy_audio_Sequence* self,
	psy_audio_OrderIndex index)
{
	const psy_audio_SequenceEntry* entry;

	assert(self);

	entry = psy_audio_sequence_entry_const(self, index);
	if (entry && entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
		return ((const psy_audio_SequencePatternEntry*)entry)->patternslot;
	}
	return psy_INDEX_INVALID;
}

psy_dsp_big_beat_t psy_audio_sequence_offset(const psy_audio_Sequence* self,
	psy_audio_OrderIndex index)
{
	const psy_audio_SequenceEntry* seqentry;

	assert(self);

	seqentry = psy_audio_sequence_entry_const(self, index);
	if (seqentry) {
		return psy_audio_sequenceentry_offset(seqentry);
	}
	return (psy_dsp_big_beat_t)0.0;
}

void psy_audio_sequence_swaptracks(psy_audio_Sequence* self,
	uintptr_t src_id, uintptr_t dst_id)
{		
	psy_audio_SequenceTrack* src;
	psy_audio_SequenceTrack* dst;

	assert(self);
	
	if (src_id == dst_id) {
		return;
	}
	src = psy_audio_sequence_track_at(self, src_id);
	dst = psy_audio_sequence_track_at(self, dst_id);
	if (src && dst) {
		psy_audio_sequence_set_track(self, src, dst_id);
		psy_audio_sequence_set_track(self, dst, src_id);	
		psy_signal_emit(&self->signal_trackswap, self, 2, src_id, dst_id);
		psy_signal_emit(&self->signal_changed, self, 0);		
	}
}

psy_audio_OrderIndex psy_audio_sequence_reorder(psy_audio_Sequence* self,
	psy_audio_OrderIndex index, psy_dsp_big_beat_t newposition)
{	
	psy_audio_SequenceTrack* track;	

	assert(self);	

	track = psy_audio_sequence_track_at(self, index.track);
	if (track && track->nodes) {
		psy_List* selected;

		selected = psy_list_at(track->nodes, index.order);
		if (selected) {
			psy_List* p;
			psy_dsp_big_beat_t curroffset;
			uintptr_t row;

			curroffset = 0.0;
			for (p = track->nodes, row = 0, curroffset = 0.0; p != NULL;
					p = p->next) {				
				psy_audio_SequenceEntry* entry;

				entry = (psy_audio_SequenceEntry*)p->entry;
				curroffset += entry->repositionoffset + 
					psy_audio_sequenceentry_length(entry);
				if (newposition < curroffset) {
					break;
				}
				++row;				
			}
			if (p != selected) {
				psy_audio_OrderIndex insertindex;

				if (!p) {
					p = track->nodes->tail;
				}
				psy_list_swap_entries(selected, p);
				psy_audio_sequence_reposition_track(self, track);
				insertindex = psy_audio_orderindex_make(index.track, row);
				psy_signal_emit(&self->signal_insert, self, 1, 
					&insertindex);
				psy_signal_emit(&self->signal_changed, self, 0);
				return insertindex;
			}
		}				
	}
	return psy_audio_orderindex_make_invalid();
}

void psy_audio_sequence_resetpatterns(psy_audio_Sequence* self)
{
	uintptr_t t;	

	assert(self);
	
	for (t = 0; t < psy_audio_sequence_width(self); ++t) {
		psy_audio_SequenceTrack* track;
		psy_List* p;

		track = psy_audio_sequence_track_at(self, t);
		if (!track) {
			continue;
		}
		for (p = track->nodes; p != NULL; psy_list_next(&p)) {
			psy_audio_SequenceEntry* entry;			

			entry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (entry && entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
				/* todo default lines */
				psy_audio_SequencePatternEntry* seqpatternentry;

				seqpatternentry = (psy_audio_SequencePatternEntry*)entry;
				psy_audio_patterns_insert(self->patterns,
					seqpatternentry->patternslot, psy_audio_pattern_allocinit());
			}
		}
	}
}

void psy_audio_sequence_fillpatterns(psy_audio_Sequence* self)
{
	uintptr_t t;

	assert(self);

	for (t = 0; t < psy_audio_sequence_width(self); ++t) {
		psy_audio_SequenceTrack* track;
		psy_List* p;

		track = psy_audio_sequence_track_at(self, t);
		if (!track) {
			continue;
		}
		for (p = track->nodes; p != NULL; psy_list_next(&p)) {
			psy_audio_SequenceEntry* entry;

			entry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (entry && entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
				/* todo default lines */
				psy_audio_SequencePatternEntry* seqpatternentry;
				seqpatternentry = (psy_audio_SequencePatternEntry*)entry;
				if (!psy_audio_patterns_at(self->patterns,
						seqpatternentry->patternslot)) {
					psy_audio_patterns_insert(self->patterns,
						seqpatternentry->patternslot, psy_audio_pattern_allocinit());
				}
			}
		}
	}
}

void psy_audio_sequence_blockremove(psy_audio_Sequence* self,
	psy_audio_BlockSelection selection)
{
	psy_audio_PatternEntryRemoveCommand command;

	psy_audio_patternentryremovecommand_init(&command);
	psy_audio_sequence_block_traverse(self, selection, &command.command);
	psy_command_dispose(&command.command);
}

void psy_audio_sequence_blocktranspose(psy_audio_Sequence* self,
	psy_audio_BlockSelection selection, intptr_t offset)
{
	psy_audio_PatternEntryTransposeCommand command;

	psy_audio_patternentrytransposecommand_init(&command, offset);
	psy_audio_sequence_block_traverse(self, selection, &command.command);
	psy_command_dispose(&command.command);
}

void psy_audio_sequence_blockcopypattern(psy_audio_Sequence* self,
	psy_audio_BlockSelection selection,
	psy_audio_Pattern* dest)
{
	psy_audio_PatternEntryCopyCommand command;

	assert(dest);
	
	psy_audio_patternentrycopycommand_init(&command, dest,
		selection.topleft.absoffset, selection.topleft.track);
	psy_audio_pattern_clear(dest);
	psy_audio_pattern_setmaxsongtracks(dest,
		selection.bottomright.track -
		selection.topleft.track);
	psy_audio_pattern_setlength(dest,
		(psy_dsp_big_beat_t)(selection.bottomright.absoffset -
			selection.topleft.absoffset));
	psy_audio_sequence_block_traverse(self, selection, &command.command);
	psy_command_dispose(&command.command);	
}

void psy_audio_sequence_blockpastepattern(psy_audio_Sequence* self,
	psy_audio_BlockSelection selection,
	psy_audio_Pattern* source)
{
	
}

void psy_audio_sequence_block_traverse(psy_audio_Sequence* self,
	psy_audio_BlockSelection selection, psy_Command* command)
{
	psy_List* iters;
	psy_audio_SequenceTrackNode* p;
	uintptr_t trackindex;
	psy_audio_SequenceCursor begin;
	psy_audio_SequenceCursor end;

	assert(self);
	assert(command);

	begin = selection.topleft;
	end = selection.bottomright;
	trackindex = 0;
	iters = NULL;	
	for (; trackindex < psy_audio_sequence_num_tracks(self); ++trackindex) {
		if (trackindex >= begin.orderindex.track && trackindex <= end.orderindex.track) {
			psy_audio_SequenceTrackIterator* ite;

			ite = psy_audio_sequencetrackiterator_allocinit();
			if (ite) {				
				psy_audio_sequence_begin(self, trackindex,
					psy_audio_sequencecursor_offset_abs(&begin), ite);
				psy_list_append(&iters, ite);
			}
		}
	}
	for (p = iters; p != NULL; psy_list_next(&p)) {
		psy_audio_SequenceTrackIterator* ite;
		psy_audio_SequenceEntry* seqentry;

		ite = (psy_audio_SequenceTrackIterator*)p->entry;
		seqentry = psy_audio_sequencetrackiterator_entry(ite);
		while (ite->patternnode &&
			psy_audio_sequencetrackiterator_offset(ite) <
			psy_audio_sequencecursor_offset_abs(&end)) {
			psy_audio_PatternEntry* entry;

			entry = (psy_audio_PatternEntry*)psy_list_entry(ite->patternnode);
			if (entry->track >= begin.track && entry->track < end.track) {
				psy_audio_PatternNode* node;
				psy_audio_Pattern* pattern;
				psy_Property params;

				node = ite->patternnode;
				pattern = ite->pattern;
				psy_property_init(&params);
				psy_property_append_int(&params, "node",(intptr_t)node, 0, 0);
				psy_property_append_int(&params, "pattern", (intptr_t)pattern, 0, 0);
				psy_audio_sequencetrackiterator_inc(ite);
				psy_command_execute(command, &params);
				psy_property_dispose(&params);
			} else {
				psy_audio_sequencetrackiterator_inc(ite);
			}
		}
	}
	psy_list_deallocate(&iters, (psy_fp_disposefunc)
		psy_audio_sequencetrackiterator_dispose);
	iters = NULL;
}

void psy_audio_sequence_block_traverse_lines(psy_audio_Sequence* self,
	psy_audio_BlockSelection selection, psy_Command* command)
{
	uintptr_t track;
	uintptr_t maxlines;
	double offset;
	double seqoffset;
	double length;
	psy_audio_SequenceTrackIterator ite;
	uintptr_t line;
	double bpl;

	assert(self);
		
	bpl = psy_audio_sequencecursor_bpl(&selection.topleft);
	offset = selection.topleft.absoffset;
	psy_audio_sequencetrackiterator_init(&ite);
	psy_audio_sequence_begin(self, selection.topleft.orderindex.track, offset,
		&ite);
	length = 0.0;
	if (ite.pattern) {
		length = ite.pattern->length;
	}
	seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);
	line = cast_decimal(offset * selection.topleft.lpb);
	{
		psy_audio_SequenceTrack* track;
		double maxoffset;

		track = psy_audio_sequence_track_at(
			self, selection.topleft.orderindex.track);
		if (track) {
			maxoffset = psy_audio_sequencetrack_duration(track,
				self->patterns);
		} else {
			maxoffset = psy_audio_sequence_duration(self);
		}
		maxlines = cast_decimal(maxoffset * selection.topleft.lpb);
	}
	while (offset <= selection.bottomright.absoffset && line < maxlines) {
		bool fill;

		fill = !(offset >= seqoffset && offset < seqoffset + length) || !ite.patternnode;
		// draw trackline
		for (track = selection.topleft.track; track < selection.bottomright.track;
			++track) {
			bool hasevent = FALSE;

			while (!fill && ite.patternnode &&
				psy_audio_sequencetrackiterator_patternentry(&ite)->track <= track &&
				psy_dsp_testrange_e(
					psy_audio_sequencetrackiterator_offset(&ite),
					offset,
					bpl)) {
				psy_audio_PatternEntry* entry;

				entry = psy_audio_sequencetrackiterator_patternentry(&ite);
				if (entry->track == track) {					
					psy_Property params;
				
					psy_property_init(&params);
					psy_property_append_int(&params, "node", (intptr_t)ite.patternnode, 0, 0);
					psy_property_append_int(&params, "pattern", (intptr_t)ite.pattern, 0, 0);
					psy_property_append_int(&params, "track", (intptr_t)track, 0, 0);
					psy_command_execute(command, &params);
					psy_property_dispose(&params);
					psy_list_next(&ite.patternnode);
					hasevent = TRUE;
					break;
				}
				psy_list_next(&ite.patternnode);
			}
			if (!hasevent) {				
				psy_Property params;

				psy_property_init(&params);
				psy_property_append_int(&params, "node", (intptr_t)NULL, 0, 0);
				psy_property_append_int(&params, "pattern", (intptr_t)(intptr_t)ite.pattern, 0, 0);
				psy_property_append_int(&params, "track", (intptr_t)track, 0, 0);
				psy_command_execute(command, &params);
				psy_property_dispose(&params);			
			} else if (ite.patternnode && ((psy_audio_PatternEntry*)(ite.patternnode->entry))->track <= track) {
				fill = TRUE;
			}
		}
		// skip remaining events of the line 
		while (ite.patternnode && (offset < seqoffset + length) &&
			(psy_audio_sequencetrackiterator_offset(&ite) + psy_dsp_epsilon * 2 <
				offset + bpl)) {
			psy_list_next(&ite.patternnode);
		}
		offset += bpl;
		if (offset >= seqoffset + length) {
			// go to next seqentry or end draw 
			if (ite.sequencentrynode && ite.sequencentrynode->next) {
				psy_audio_sequencetrackiterator_inc_entry(&ite);
				seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);
				offset = seqoffset;
				if (ite.pattern) {
					length = ite.pattern->length;
				} else {
					break;
				}
			} else {
				break;
			}
		}
	}	
	psy_audio_sequencetrackiterator_dispose(&ite);
}

psy_audio_PatternEvent psy_audio_sequence_pattern_event_at_cursor(
	const psy_audio_Sequence* self, psy_audio_SequenceCursor cursor)
{
	const psy_audio_Pattern* pattern;	

	pattern = psy_audio_sequence_pattern_const(self, cursor.orderindex);
	if (pattern) {
		return psy_audio_pattern_event_at_cursor(pattern, cursor);
	}
	return psy_audio_patternevent_zero();
}
