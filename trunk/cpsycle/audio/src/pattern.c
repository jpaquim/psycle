// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "pattern.h"
// std
#include <math.h>
// platform
#include "../../detail/portable.h"

static double hermitecurveinterpolate(int kf0, int kf1, int kf2,
	int kf3, int curposition, int maxposition, double tangmult,
	bool interpolation);

// psy_audio_PatternCursor
// implementation
void psy_audio_patterncursor_init(psy_audio_PatternCursor* self)
{
	assert(self);
	self->key = psy_audio_NOTECOMMANDS_MIDDLEC;
	self->track = 0;
	self->offset = 0;
	self->line = 0;
	self->column = 0;
	self->digit = 0;
	self->patternid = 0;
}

psy_audio_PatternCursor psy_audio_patterncursor_make(
	uintptr_t track, psy_dsp_big_beat_t offset)
{
	psy_audio_PatternCursor rv;

	psy_audio_patterncursor_init(&rv);
	rv.track = track;
	rv.offset = offset;	
	return rv;
}

int psy_audio_patterncursor_equal(psy_audio_PatternCursor* lhs,
	psy_audio_PatternCursor* rhs)
{
	assert(lhs && rhs);
	return 
		rhs->column == lhs->column &&
		rhs->digit == lhs->digit &&
		rhs->track == lhs->track &&
		rhs->offset == lhs->offset &&
		rhs->patternid == lhs->patternid;
}

// PatternSelection
// implementation
void psy_audio_patternselection_init(psy_audio_PatternSelection* self)
{
	psy_audio_patterncursor_init(&self->topleft);
	psy_audio_patterncursor_init(&self->bottomright);
	self->valid = FALSE;
}

void psy_audio_patternselection_init_all(psy_audio_PatternSelection* self,
	psy_audio_PatternCursor topleft, psy_audio_PatternCursor bottomright)
{
	assert(self);

	self->topleft = topleft;
	self->bottomright = bottomright;
	self->valid = TRUE;
}

psy_audio_PatternSelection psy_audio_patternselection_make(
	psy_audio_PatternCursor topleft, psy_audio_PatternCursor bottomright)
{
	psy_audio_PatternSelection rv;

	psy_audio_patternselection_init_all(&rv, topleft, bottomright);
	rv.topleft = topleft;
	rv.bottomright = bottomright;
	return rv;
}

static uintptr_t defaultlines = 64;

// Pattern
// prototypes
static void psy_audio_pattern_init_signals(psy_audio_Pattern*);
static void psy_audio_pattern_dispose_signals(psy_audio_Pattern*);
// implementation
void psy_audio_pattern_init(psy_audio_Pattern* self)
{
	assert(self);

	self->events = NULL;
	// todo needs player lpb to be correct
	self->length = defaultlines / (psy_dsp_big_beat_t)4.0;
	self->name = strdup("Untitled");	
	self->opcount = 0;
	self->maxsongtracks = 0;
	psy_audio_pattern_init_signals(self);
}

void psy_audio_pattern_init_signals(psy_audio_Pattern* self)
{
	assert(self);

	psy_signal_init(&self->signal_namechanged);
	psy_signal_init(&self->signal_lengthchanged);
}

void psy_audio_pattern_dispose(psy_audio_Pattern* self)
{	
	assert(self);

	psy_list_deallocate(&self->events,
		(psy_fp_disposefunc)psy_audio_patternentry_dispose);
	free(self->name);
	self->name = NULL;
	psy_audio_pattern_dispose_signals(self);
}

void psy_audio_pattern_dispose_signals(psy_audio_Pattern* self)
{
	assert(self);
	psy_signal_dispose(&self->signal_namechanged);
	psy_signal_dispose(&self->signal_lengthchanged);
}

void psy_audio_pattern_copy(psy_audio_Pattern* self, psy_audio_Pattern* src)
{	
	psy_audio_PatternNode* p;
	uintptr_t opcount;

	assert(self);

	opcount = self->opcount;
	psy_audio_pattern_dispose(self);
	psy_audio_pattern_init(self);
	for (p = src->events; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEntry* srcentry;
		psy_audio_PatternEntry* entry;

		srcentry = (psy_audio_PatternEntry*)psy_list_entry(p);
		entry = psy_audio_patternentry_clone(srcentry);
		psy_list_append(&self->events, entry);
	}
	self->length = src->length;
	psy_strreset(&self->name, src->name);	
	self->opcount = opcount + 1;
}

psy_audio_Pattern* psy_audio_pattern_alloc(void)
{
	return (psy_audio_Pattern*) malloc(sizeof(psy_audio_Pattern));
}

psy_audio_Pattern* psy_audio_pattern_allocinit(void)
{
	psy_audio_Pattern* rv;

	rv = psy_audio_pattern_alloc();
	if (rv) {
		psy_audio_pattern_init(rv);
	}
	return rv;
}

psy_audio_Pattern* psy_audio_pattern_clone(psy_audio_Pattern* self)
{	
	psy_audio_Pattern* rv;	

	assert(self);
	rv = psy_audio_pattern_allocinit();
	if (rv) {
		psy_audio_pattern_copy(rv, self);
	}
	return rv;
}

void psy_audio_pattern_remove(psy_audio_Pattern* self, psy_audio_PatternNode* node)
{
	assert(self);
	if (node) {
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)(node->entry);
		psy_list_remove(&self->events, node);
		psy_audio_patternentry_dispose(entry);
		free(entry);
		++self->opcount;		
	}
}

psy_audio_PatternNode* psy_audio_pattern_insert(psy_audio_Pattern* self,
	psy_audio_PatternNode* prev, uintptr_t track, psy_dsp_big_beat_t offset,
	const psy_audio_PatternEvent* event)
{
	psy_audio_PatternNode* rv;
	psy_audio_PatternEntry* entry;

	assert(self);
	if (event) {
		entry = psy_audio_patternentry_alloc();
		psy_audio_patternentry_init_all(entry, event, offset, 0.f, 0.f, track);
		if (!self->events) {
			rv = self->events = psy_list_create(entry);
		} else {	
			rv = psy_list_insert(&self->events, prev, entry);		
		}	
		++self->opcount;
		return rv;
	}
	return NULL;
}

void psy_audio_pattern_setevent(psy_audio_Pattern* self, psy_audio_PatternNode* node,
	const psy_audio_PatternEvent* event)
{
	assert(self);
	if (node) {
		psy_audio_PatternEntry* entry;
			
		entry = (psy_audio_PatternEntry*) node->entry;
		if (event) {			
			*psy_audio_patternentry_front(entry) = *event;
		} else {
			psy_audio_PatternEvent empty;

			psy_audio_patternevent_clear(&empty);
			*psy_audio_patternentry_front(entry) = empty;
		}
		++self->opcount;
	}
}

psy_audio_PatternEvent psy_audio_pattern_event(psy_audio_Pattern* self, psy_audio_PatternNode* node)
{
	assert(self);
	if (node) {
		return *psy_audio_patternentry_front(((psy_audio_PatternEntry*)node->entry));
	} else {
		psy_audio_PatternEvent empty;

		psy_audio_patternevent_clear(&empty);
		return empty;
	}	
}

psy_audio_PatternNode* psy_audio_pattern_greaterequal(psy_audio_Pattern* self, psy_dsp_big_beat_t offset)
{
	psy_audio_PatternNode* p;

	assert(self);

	p = self->events;
	while (p != NULL) {
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)psy_list_entry(p);
		if (entry->offset >= offset) {			
			break;
		}		
		psy_list_next(&p);
	}	
	return p;
}

psy_audio_PatternNode* psy_audio_pattern_greaterequal_track(psy_audio_Pattern* self,
	uintptr_t track, psy_dsp_big_beat_t offset)
{
	psy_audio_PatternNode* p;

	assert(self);

	p = self->events;
	while (p != NULL) {
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)psy_list_entry(p);
		if (entry->track == track && entry->offset >= offset) {
			break;
		}
		psy_list_next(&p);
	}
	return p;
}

psy_audio_PatternNode* psy_audio_pattern_findnode(psy_audio_Pattern* self, uintptr_t track,
	psy_dsp_big_beat_t offset, psy_dsp_big_beat_t bpl, psy_audio_PatternNode** prev)
{
	psy_audio_PatternNode* node;
	
	assert(self);

	node = psy_audio_pattern_greaterequal(self, offset);
	if (node) {
		*prev = node->prev;
	} else {		
		*prev = psy_audio_pattern_last(self);
	}	
	while (node) {
		psy_audio_PatternEntry* entry;
		
		entry = (psy_audio_PatternEntry*)(node->entry);
		if (entry->offset >= offset + bpl || entry->track > track) {
			node = NULL;
			break;
		}		
		if (entry->track == track) {
			break;
		}				
		*prev = node;		
		psy_list_next(&node);
	}
	return node;
}

psy_audio_PatternNode* psy_audio_pattern_last(psy_audio_Pattern* self)
{	
	assert(self);

	return psy_list_last(self->events);
}

psy_audio_PatternNode* psy_audio_pattern_next_track(psy_audio_Pattern* self,
	psy_audio_PatternNode* node, uintptr_t track)
{	
	if (node) {
		psy_audio_PatternNode* rv;

		rv = node->next;
		while (rv) {
			psy_audio_PatternEntry* entry;

			entry = psy_audio_patternnode_entry(rv);
			if (entry->track == track) {
				break;
			}
			rv = rv->next;
		}
		return rv;
	}
	return NULL;
}

psy_audio_PatternNode* psy_audio_pattern_prev_track(psy_audio_Pattern* self,
	psy_audio_PatternNode* node, uintptr_t track)
{
	if (node) {
		psy_audio_PatternNode* rv;

		rv = node->prev;
		while (rv) {
			psy_audio_PatternEntry* entry;

			entry = psy_audio_patternnode_entry(rv);
			if (entry->track == track) {
				break;
			}
			rv = rv->prev;
		}
		return rv;
	}
	return NULL;
}

void psy_audio_pattern_setname(psy_audio_Pattern* self, const char* text)
{
	assert(self);

	psy_strreset(&self->name, text);
	++self->opcount;
	psy_signal_emit(&self->signal_namechanged, self, 0);
}

void psy_audio_pattern_setlength(psy_audio_Pattern* self, psy_dsp_big_beat_t length)
{
	assert(self);

	self->length = length;
	++self->opcount;
	psy_signal_emit(&self->signal_lengthchanged, self, 0);
}

void psy_audio_pattern_scale(psy_audio_Pattern* self, float factor)
{
	psy_audio_PatternNode* p;	
	
	assert(self);

	for (p = self->events; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)psy_list_entry(p);

		entry->offset *= factor;
	}
}

void psy_audio_pattern_blockremove(psy_audio_Pattern* self,
	psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end)
{	
	psy_audio_PatternNode* p;
	psy_audio_PatternNode* q;

	assert(self);

	p = psy_audio_pattern_greaterequal(self, (psy_dsp_big_beat_t) begin.offset);
	while (p != NULL) {			
		psy_audio_PatternEntry* entry;
		q = p->next;

		entry = (psy_audio_PatternEntry*)psy_list_entry(p);
		if (entry->offset < end.offset) {
			if (entry->track >= begin.track && entry->track < end.track) {
				psy_audio_pattern_remove(self, p);
			}
		} else {
			break;
		}
		p = q;
	}
}

void psy_audio_pattern_blockinterpolatelinear(psy_audio_Pattern* self,
	psy_audio_PatternSelection selection, psy_dsp_big_beat_t bpl)
{
	intptr_t startval;
	intptr_t endval;
	psy_audio_PatternNode* prev;
	psy_audio_PatternNode* node;
	psy_audio_PatternCursor begin;
	psy_audio_PatternCursor end;

	assert(self);

	begin = selection.topleft;
	end = selection.bottomright;
	begin.line = (uintptr_t)(begin.offset / bpl);
	end.line = (uintptr_t)(end.offset / bpl);
	node = psy_audio_pattern_findnode(self, begin.track, begin.line * bpl, bpl, &prev);
	startval = (node)
		? psy_audio_patternevent_tweakvalue(psy_audio_patternentry_front(node->entry))
		: 0;
	node = psy_audio_pattern_findnode(self, begin.track, (end.line - 1) * bpl, bpl, &prev);
	endval = (node)
		? psy_audio_patternevent_tweakvalue(psy_audio_patternentry_front(node->entry))
		: 0;
	psy_audio_pattern_blockinterpolaterange(self, begin, end, bpl, startval, endval);
}

void psy_audio_pattern_blockinterpolaterange(psy_audio_Pattern* self,
	psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end,
	psy_dsp_big_beat_t bpl, intptr_t startval, intptr_t endval)
{
	uintptr_t line;
	float step;
	psy_audio_PatternNode* prev = 0;
	psy_audio_PatternNode* node;

	assert(self);

	step = (endval - startval) / (float)(end.line - begin.line - 1);
	for (line = begin.line; line < end.line; ++line) {
		psy_dsp_big_beat_t offset;
		int value;

		offset = line * bpl;
		value = (int)((step * (line - begin.line) + startval));
		node = psy_audio_pattern_findnode(self, begin.track, offset, bpl, &prev);
		if (node) {
			psy_audio_patternevent_settweakvalue(psy_audio_patternentry_front(
				node->entry), value);
			++self->opcount;
		}
		else {
			psy_audio_PatternEvent ev;

			psy_audio_patternevent_clear(&ev);
			psy_audio_patternevent_settweakvalue(&ev, value);
			prev = psy_audio_pattern_insert(self, prev, begin.track, offset, &ev);
		}
	}
}

void psy_audio_pattern_blockinterpolaterangehermite(psy_audio_Pattern* self,
	psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end,
	psy_dsp_big_beat_t bpl, intptr_t startval, intptr_t endval)
{
	uintptr_t line;
	psy_audio_PatternNode* prev = 0;
	psy_audio_PatternNode* node;
	int val0 = 0;
	int val1 = 0;
	int val2 = 0;
	int val3 = 0;
	int distance;

	assert(self);

	val0 = startval;
	val1 = startval;
	val2 = endval;
	val3 = endval;
	distance = end.line - begin.line - 1;
	for (line = begin.line; line < end.line; ++line) {
		psy_dsp_big_beat_t offset;

		offset = line * bpl;
		node = psy_audio_pattern_findnode(self, begin.track, offset, bpl, &prev);
		if (node) {
			double curveval = hermitecurveinterpolate(val0, val1, val2, val3, line - begin.line, distance, 0, TRUE);
			psy_audio_patternevent_settweakvalue(psy_audio_patternentry_front(node->entry), (intptr_t)curveval);
			++self->opcount;
		} else {
			psy_audio_PatternEvent ev;
			double curveval = hermitecurveinterpolate(val0, val1, val2, val3, line - begin.line, distance, 0, TRUE);

			psy_audio_patternevent_clear(&ev);
			psy_audio_patternevent_settweakvalue(&ev, (intptr_t)curveval);
			prev = psy_audio_pattern_insert(self, prev, begin.track, offset, &ev);
		}
	}
}

double hermitecurveinterpolate(int kf0, int kf1, int kf2,
	int kf3, int curposition, int maxposition, double tangmult, bool interpolation)
{
	if (interpolation == TRUE) {
		double s = (double)curposition / (double)maxposition;
		double pws3 = pow(s, 3);
		double pws2 = pow(s, 2);
		double pws23 = 3 * pws2;
		double h1 = (2 * pws3) - pws23 + 1;
		double h2 = (-2 * pws3) + pws23;
		double h3 = pws3 - (2 * pws2) + s;
		double h4 = pws3 - pws2;

		double t1 = tangmult * (kf2 - kf0);
		double t2 = tangmult * (kf3 - kf1);

		return (h1 * kf1 + h2 * kf2 + h3 * t1 + h4 * t2);
	} else {
		return kf1;
	}
}

void psy_audio_pattern_blocktranspose(psy_audio_Pattern* self,
	psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end, int offset)
{	
	psy_audio_PatternNode* p;
	psy_audio_PatternNode* q;

	assert(self);

	p = psy_audio_pattern_greaterequal(self, (psy_dsp_big_beat_t) begin.offset);
	while (p != NULL) {			
		psy_audio_PatternEntry* entry;
		q = p->next;

		entry = (psy_audio_PatternEntry*)psy_list_entry(p);
		if (entry->offset < end.offset) {
			if (entry->track >= begin.track && entry->track < end.track) {
				if (psy_audio_patternentry_front(entry)->note < psy_audio_NOTECOMMANDS_RELEASE) {
					if (psy_audio_patternentry_front(entry)->note + offset < 0) {
						psy_audio_patternentry_front(entry)->note = 0;
					} else {
						psy_audio_patternentry_front(entry)->note += offset;
					}					
					if (psy_audio_patternentry_front(entry)->note >= psy_audio_NOTECOMMANDS_RELEASE) {
						psy_audio_patternentry_front(entry)->note = psy_audio_NOTECOMMANDS_RELEASE - 1;
					}
				}
			}
		} else {
			break;
		}	
		p = q;
	}
	++self->opcount;
}

void psy_audio_pattern_changemachine(psy_audio_Pattern* self,
	psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end, int machine)
{
	psy_audio_PatternNode* p;
	psy_audio_PatternNode* q;

	assert(self);

	p = psy_audio_pattern_greaterequal(self, (psy_dsp_big_beat_t) begin.offset);
	while (p != NULL) {			
		psy_audio_PatternEntry* entry;
		q = p->next;

		entry = (psy_audio_PatternEntry*)psy_list_entry(p);
		if (entry->offset < end.offset) {
			if (entry->track >= begin.track && entry->track < end.track) {
				psy_audio_patternentry_front(entry)->mach = machine;
			}
		} else {
			break;
		}	
		p = q;
	}
}

void psy_audio_pattern_changeinstrument(psy_audio_Pattern* self,
	psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end, int instrument)
{
	psy_audio_PatternNode* p;
	psy_audio_PatternNode* q;

	assert(self);

	p = psy_audio_pattern_greaterequal(self, (psy_dsp_big_beat_t) begin.offset);
	while (p != NULL) {			
		psy_audio_PatternEntry* entry;
		q = p->next;

		entry = (psy_audio_PatternEntry*)psy_list_entry(p);
		if (entry->offset < end.offset) {
			if (entry->track >= begin.track && entry->track < end.track) {
				psy_audio_patternentry_front(entry)->inst = instrument;
			}
		} else {
			break;
		}	
		p = q;
	}
}

void psy_audio_pattern_setmaxsongtracks(psy_audio_Pattern* self, uintptr_t num)
{
	assert(self);

	self->maxsongtracks = num;
}

uintptr_t psy_audio_pattern_maxsongtracks(const psy_audio_Pattern* self)
{
	assert(self);

	return self->maxsongtracks;
}

psy_audio_PatternCursor psy_audio_pattern_searchinpattern(psy_audio_Pattern*
	self, psy_audio_PatternSelection selection, psy_audio_PatternSearchReplaceMode mode)
{
	psy_audio_PatternCursor cursor;
	psy_audio_PatternNode* p;
	psy_audio_PatternNode* q;
	const psy_audio_fp_matches notematcher = mode.notematcher;
	const psy_audio_fp_matches instmatcher = mode.instmatcher;
	const psy_audio_fp_matches machmatcher = mode.machmatcher;
	const uintptr_t notereference = mode.notereference;
	const uintptr_t instreference = mode.instreference;
	const uintptr_t machreference = mode.machreference;

	assert(self);

	cursor.offset = -1;
	cursor.line = UINTPTR_MAX;
	p = psy_audio_pattern_greaterequal(self, (psy_dsp_big_beat_t)selection.topleft.offset);
	while (p != NULL) {
		psy_audio_PatternEntry* entry;
		q = p->next;

		entry = (psy_audio_PatternEntry*)psy_list_entry(p);
		if (entry->offset < selection.bottomright.offset) {
			if (entry->track >= selection.topleft.track && entry->track < selection.bottomright.track) {
				psy_audio_PatternEvent* patternevent;

				patternevent = psy_audio_patternentry_front(entry);
				if (notematcher(patternevent->note, notereference)
					&& instmatcher(patternevent->inst, instreference)
					&& machmatcher(patternevent->mach, machreference)) {
					cursor.offset = entry->offset;
					cursor.track = entry->track;
					cursor.column = 0;					
					cursor.key = patternevent->note;
					cursor.line = 0;
					cursor.patternid = 0;
					cursor.digit = 0;
					return cursor;
				}
			}
		} else {
			break;
		}
		p = q;
	}
	return cursor;
}


// psy_audio_PatternCursorNavigator
// implementation
bool psy_audio_patterncursornavigator_advancelines(psy_audio_PatternCursorNavigator*
	self, uintptr_t lines)
{
	psy_dsp_big_beat_t maxlength;

	assert(self);
	assert(self->cursor);
	assert(self->pattern);

	maxlength = psy_audio_pattern_length(self->pattern);
	if (lines > 0) {
		int currlines;

		currlines = cast_decimal(self->cursor->offset / self->bpl);
		self->cursor->offset = (currlines + lines) * self->bpl;				
		if (self->cursor->offset >= maxlength) {
			if (self->wrap) {
				self->cursor->offset = self->cursor->offset - maxlength;
				if (self->cursor->offset > maxlength - self->bpl) {
					self->cursor->offset = maxlength - self->bpl;
				}
				self->cursor->line = cast_decimal(self->cursor->offset / self->bpl);
				return FALSE;
			} else {
				self->cursor->offset = maxlength - self->bpl;
			}
		}
		self->cursor->line = cast_decimal(self->cursor->offset / self->bpl);
	}
	return TRUE;
}

bool psy_audio_patterncursornavigator_prevlines(
	psy_audio_PatternCursorNavigator* self, uintptr_t lines)
{
	psy_dsp_big_beat_t maxlength;

	assert(self);
	assert(self->cursor);
	assert(self->pattern);	

	maxlength = psy_audio_pattern_length(self->pattern);
	if (lines > 0) {
		int currlines;

		currlines = cast_decimal(self->cursor->offset / self->bpl);
		self->cursor->offset = (currlines - (intptr_t)lines) * self->bpl;		
		if (self->cursor->offset < 0.0) {
			if (self->wrap) {
				self->cursor->offset += maxlength;
				if (self->cursor->offset < 0) {
					self->cursor->offset = 0.0;
					self->cursor->line = 0;
				}					
				self->cursor->line = cast_decimal(self->cursor->offset / self->bpl);
				return TRUE;
			} else {
				self->cursor->offset = 0.0;
				self->cursor->line = 0;
			}
		}
	}
	self->cursor->line = cast_decimal(self->cursor->offset / self->bpl);
	return FALSE;
}

bool psy_audio_patterncursornavigator_advancekeys(psy_audio_PatternCursorNavigator*
	self, uintptr_t lines)
{
	uintptr_t maxkey;

	assert(self);
	assert(self->cursor);
	assert(self->pattern);

	maxkey = self->maxkeys;
	if (lines > 0) {
		self->cursor->key += lines;
		if (self->cursor->key >= maxkey) {
			if (self->wrap) {
				self->cursor->key = self->cursor->key - maxkey;
				if (self->cursor->key > maxkey - 1) {
					self->cursor->key = maxkey - 1;
				}
				return FALSE;
			} else {
				self->cursor->key = maxkey - 1;
			}
		}
	}
	return TRUE;
}

bool psy_audio_patterncursornavigator_prevkeys(
	psy_audio_PatternCursorNavigator* self, uintptr_t lines)
{
	uintptr_t maxkey;

	assert(self);
	assert(self->cursor);
	assert(self->pattern);

	maxkey = self->maxkeys;
	if (lines > 0) {
		if (self->cursor->key >= lines) {
			self->cursor->key -= lines;					
		} else {
			if (self->wrap) {
				self->cursor->key += maxkey;
				return TRUE;
			} else {
				self->cursor->key = 0;
			}
		}
	}
	return FALSE;
}

bool psy_audio_patterncursornavigator_prevtrack(
	psy_audio_PatternCursorNavigator* self,
	uintptr_t numsongtracks)
{	
	self->cursor->column = 0;
	self->cursor->digit = 0;
	if (self->cursor->track > 0) {
		--self->cursor->track;
		return TRUE;		
	} else if (self->wrap) {
		self->cursor->track = numsongtracks - 1;
		return FALSE;		
	}
	return TRUE;
}

bool psy_audio_patterncursornavigator_nexttrack(
	psy_audio_PatternCursorNavigator* self,
	uintptr_t numsongtracks)
{
	assert(self);

	self->cursor->column = 0;
	self->cursor->digit = 0;
	if (self->cursor->track < numsongtracks - 1) {
		++self->cursor->track;
		return FALSE;		
	} else if (self->wrap) {
		self->cursor->track = 0;
		return TRUE;
	}
	return FALSE;
}

void psy_audio_pattern_blockcopy(psy_audio_Pattern* self,
	psy_audio_Pattern* source, psy_audio_PatternSelection selection)
{
	psy_audio_PatternNode* begin;
	psy_audio_PatternNode* p;
	psy_audio_PatternNode* q;
	psy_audio_PatternNode* prev = 0;
	psy_dsp_big_beat_t offset;
	int trackoffset;
	int restore_opcount;

	assert(self);

	restore_opcount = self->opcount;
	begin = psy_audio_pattern_greaterequal(source,
		(psy_dsp_big_beat_t)selection.topleft.offset);
	offset = (psy_dsp_big_beat_t)selection.topleft.offset;
	trackoffset = selection.topleft.track;
	psy_audio_pattern_dispose(self);
	psy_audio_pattern_init(self);
	p = begin;
	while (p != NULL) {
		psy_audio_PatternEntry* entry;
		q = p->next;

		entry = psy_audio_patternnode_entry(p);
		if (entry->offset < selection.bottomright.offset) {
			if (entry->track >= selection.topleft.track &&
				entry->track < selection.bottomright.track) {
				prev = psy_audio_pattern_insert(self,
					prev, entry->track - trackoffset,
					entry->offset - offset,
					psy_audio_patternentry_front(entry));
			}
		} else {
			break;
		}
		p = q;
	}
	psy_audio_pattern_setmaxsongtracks(self,
		selection.bottomright.track -
		selection.topleft.track);
	psy_audio_pattern_setlength(self,
		(psy_dsp_big_beat_t)(selection.bottomright.offset -
			selection.topleft.offset));
	self->opcount = restore_opcount + 1;
}

void psy_audio_pattern_blockpaste(psy_audio_Pattern* self,
	psy_audio_Pattern* source, psy_audio_PatternCursor destcursor,
	psy_dsp_big_beat_t bpl)
{
	psy_audio_PatternNode* p;
	psy_audio_PatternNode* prev = 0;
	psy_dsp_big_beat_t offset;
	int trackoffset;
	psy_audio_PatternCursor begin;
	psy_audio_PatternCursor end;

	assert(self);

	offset = (psy_dsp_big_beat_t)destcursor.offset;
	trackoffset = destcursor.track;
	p = source->events;

	begin = end = destcursor;
	end.track += source->maxsongtracks;
	end.offset += source->length;
	if (end.offset >= psy_audio_pattern_length(self)) {
		end.offset = psy_audio_pattern_length(self);
	}
	psy_audio_pattern_blockremove(self, begin, end);
	// sequencer_checkiterators(&workspace_player(self->workspace).sequencer,
	//	node);
	while (p != NULL) {
		psy_audio_PatternEntry* pasteentry;
		psy_audio_PatternNode* node;

		pasteentry = psy_audio_patternnode_entry(p);
		node = psy_audio_pattern_findnode(self,
			pasteentry->track + trackoffset,
			pasteentry->offset + offset,
			(psy_dsp_big_beat_t)bpl,
			&prev);
		if (node) {
			psy_audio_PatternEntry* entry;

			entry = (psy_audio_PatternEntry*)node->entry;
			*psy_audio_patternentry_front(entry) =
				*psy_audio_patternentry_front(pasteentry);
		} else {
			psy_audio_pattern_insert(self,
				prev,
				pasteentry->track + trackoffset,
				pasteentry->offset + offset,
				psy_audio_patternentry_front(pasteentry));
		}
		p = p->next;
	}	
}

void psy_audio_pattern_blockmixpaste(psy_audio_Pattern* self,
	psy_audio_Pattern* source, psy_audio_PatternCursor destcursor,
	psy_dsp_big_beat_t bpl)
{
	psy_audio_PatternNode* p;
	psy_audio_PatternNode* prev = 0;
	psy_dsp_big_beat_t offset;
	uintptr_t trackoffset;
	psy_audio_PatternCursor begin;
	psy_audio_PatternCursor end;

	assert(self);

	offset = (psy_dsp_big_beat_t)destcursor.offset;
	trackoffset = destcursor.track;
	begin = end = destcursor;
	end.track += source->maxsongtracks;
	end.offset += source->length;
	if (end.offset >= psy_audio_pattern_length(self)) {
		end.offset = psy_audio_pattern_length(self);
	}
	p = source->events;
	while (p != NULL) {
		psy_audio_PatternEntry* pasteentry;

		pasteentry = psy_audio_patternnode_entry(p);
		if (!psy_audio_pattern_findnode(self,
			pasteentry->track + trackoffset,
			pasteentry->offset + offset,
			(psy_dsp_big_beat_t)bpl,
			&prev)) {
			psy_audio_pattern_insert(self,
				prev,
				pasteentry->track + trackoffset,
				pasteentry->offset + offset,
				psy_audio_patternentry_front(pasteentry));
		}
		psy_audio_patternnode_next(&p);
	}	
}

void psy_audio_pattern_swingfill(psy_audio_Pattern* self,
	psy_audio_PatternCursor begin,
	psy_audio_PatternCursor end,
	bool bTrackMode,
	psy_dsp_big_beat_t bpl,
	int tempo, int width, float variance, float phase,
	bool offset)
{					
	const float twopi = 2.0f * psy_dsp_PI_F;
	uintptr_t line;	
	psy_audio_PatternNode* prev = 0;
	psy_audio_PatternNode* node;
	uintptr_t l;
	float var = (variance / 100.0f);
	// time to do our fill
	// first some math
	// our range has to go from spd+var to spd-var and back in width+1 lines
	float step = twopi / (width);
	float index = phase * twopi / 360;
	float dcoffs = 0;

	begin.line = (uintptr_t)(begin.offset / bpl);
	end.line = (uintptr_t)(end.offset / bpl);
	if (bTrackMode) {		
		begin.line = 0;
		end.line = (int)(psy_audio_pattern_length(self) / bpl);		
	}	
	// remember we are at each speed for the length of time it takes to do one tick
	// this approximately calculates the offset	
	if (offset) {
		float swing = 0;
		for (l = 0; l < width; l++) {
			float val = ((sinf(index) * var * tempo) + tempo);
			swing += (val / tempo) * (val / tempo);
			index += step;
		}
		dcoffs = ((swing - width) * tempo) / width;
	}
	// now fill the pattern
	for (line = begin.line; line < end.line; ++line) {
		psy_dsp_big_beat_t offset;
		
		offset = line * bpl;
		// -0x20; // ***** proposed change to ffxx command to allow
		// moreuseable range since the tempo bar only uses this range anyway...
		int val = (int)((((sinf(index) * var * tempo) + tempo) + dcoffs) + 0.5f);
		if (val < 1) {
			val = 1;
		} else if (val > 255) {
			val = 255;
		}
		index += step;
		node = psy_audio_pattern_findnode(self, begin.track, offset, bpl,
			&prev);				
		if (node) {
			psy_audio_PatternEntry* entry;
			psy_audio_PatternEvent* ev;

			entry = node->entry;			
			ev = psy_audio_patternentry_front(node->entry);
			ev->cmd = 0xff;
			ev->parameter = (unsigned char)(val);
			++self->opcount;										
		} else {
			psy_audio_PatternEvent ev;

			psy_audio_patternevent_clear(&ev);
			ev.cmd = 0xff;
			ev.parameter = (unsigned char)(val);
			prev = psy_audio_pattern_insert(self, prev, begin.track, offset, &ev);
		}		
	}
}

void psy_audio_pattern_setdefaultlines(uintptr_t numlines)
{
	defaultlines = numlines;
}

uintptr_t psy_audio_pattern_defaultlines(void)
{
	return defaultlines;
}