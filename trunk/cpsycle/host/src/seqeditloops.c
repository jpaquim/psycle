/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditloops.h"
/* host */
#include "styles.h"
/* audio */
#include <exclusivelock.h>
/* platform */
#include "../../detail/portable.h"

/* SeqEditLoops*/
/* prototypes */
static void seqeditloops_ondestroy(SeqEditLoops*);
static void seqeditloops_ondraw(SeqEditLoops*, psy_ui_Graphics*);
static void seqeditloops_drawloops(SeqEditLoops*, psy_ui_Graphics*);
static void seqeditloops_drawloop(SeqEditLoops*, psy_ui_Graphics*,
	psy_dsp_big_beat_t offset, int cmd, int parameter);
static void seqeditloops_onmousedown(SeqEditLoops*, psy_ui_MouseEvent*);
static void seqeditloops_onmousemove(SeqEditLoops*, psy_ui_MouseEvent*);
static void seqeditloops_onmouseup(SeqEditLoops*, psy_ui_MouseEvent*);
static void seqeditloops_onmouseenter(SeqEditLoops*);
static void seqeditloops_onmouseleave(SeqEditLoops*);
static void seqeditloops_onmousedoubleclick(SeqEditLoops*, psy_ui_MouseEvent*);
static void seqeditloops_onpreferredsize(SeqEditLoops*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static psy_audio_PatternNode* seqeditloops_findloopstart(SeqEditLoops*,
	psy_audio_PatternNode* end);
static psy_audio_PatternNode* seqeditloops_findloopend(SeqEditLoops*,
	psy_audio_PatternNode* begin);
/* vtable */
static psy_ui_ComponentVtable seqeditloops_vtable;
static bool seqeditloops_vtable_initialized = FALSE;

static void seqeditloops_vtable_init(SeqEditLoops* self)
{
	if (!seqeditloops_vtable_initialized) {
		seqeditloops_vtable = *(self->component.vtable);
		seqeditloops_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqeditloops_ondestroy;
		seqeditloops_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			seqeditloops_ondraw;
		seqeditloops_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqeditloops_onpreferredsize;
		seqeditloops_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditloops_onmousedown;
		seqeditloops_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqeditloops_onmousemove;
		seqeditloops_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			seqeditloops_onmouseup;
		seqeditloops_vtable.onmouseenter =
			(psy_ui_fp_component_onmouseenter)
			seqeditloops_onmouseenter;
		seqeditloops_vtable.onmouseleave =
			(psy_ui_fp_component_onmouseleave)
			seqeditloops_onmouseleave;
		seqeditloops_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			seqeditloops_onmousedoubleclick;
		seqeditloops_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqeditloops_vtable;
}
/* implementation */
void seqeditloops_init(SeqEditLoops* self, psy_ui_Component* parent,
	SeqEditState* state)
{
	assert(self);
	assert(state);

	psy_ui_component_init(&self->component, parent, parent);
	seqeditloops_vtable_init(self);
	psy_signal_init(&self->signal_changed);
	self->state = state;	
	self->drag = FALSE;
	psy_ui_component_doublebuffer(&self->component);	
}

void seqeditloops_ondestroy(SeqEditLoops* self)
{
	psy_signal_dispose(&self->signal_changed);
}

void seqeditloops_ondraw(SeqEditLoops* self, psy_ui_Graphics* g)
{
	seqeditloops_drawloops(self, g);
}

void seqeditloops_drawloops(SeqEditLoops* self, psy_ui_Graphics* g)
{
	psy_audio_Sequence* sequence;

	sequence = seqeditstate_sequence(self->state);
	if (sequence && sequence->patterns) {
		psy_List* p;

		for (p = sequence->globaltrack.entries; p != NULL; p = p->next) {
			psy_audio_SequenceEntry* seqentry;

			seqentry = (psy_audio_SequenceEntry*)p->entry;
			if (seqentry && seqentry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
				psy_audio_SequencePatternEntry* seqpatternentry;
				psy_audio_Pattern* pattern;

				seqpatternentry = (psy_audio_SequencePatternEntry*)seqentry;
				pattern = psy_audio_sequencepatternentry_pattern(seqpatternentry,
					sequence->patterns);
				if (pattern) {
					psy_audio_PatternNode* node;

					for (node = pattern->events; node != NULL; node = node->next) {
						psy_audio_PatternEntry* patternentry;
						psy_audio_PatternEvent* e;

						patternentry = (psy_audio_PatternEntry*)node->entry;
						e = psy_audio_patternentry_front(patternentry);
						if (e->cmd == 0xFE && (e->parameter & 0xF0) == 0xB0) {
							seqeditloops_drawloop(self, g, patternentry->offset, e->cmd,
								e->parameter);
						}
					}
				}
			}
		}
	}
}

void seqeditloops_drawloop(SeqEditLoops* self, psy_ui_Graphics* g,
	psy_dsp_big_beat_t offset, int cmd, int parameter)
{
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;
	double cpx;
	double barheight;
	double vcenter;
	double vcentertext;
	int repeat;
		
	tm = psy_ui_component_textmetric(&self->component);
	cpx = seqeditstate_beattopx(self->state, offset);
	size = psy_ui_component_size_px(&self->component);
	repeat = parameter & 0x0F;
	barheight = floor(tm->tmHeight);
	vcenter = (size.height - barheight) / 2;
	vcentertext = (size.height - tm->tmHeight) / 2;
	if (repeat == 0) {
		psy_ui_RealRectangle r1;
		psy_ui_RealRectangle r2;

		r1 = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(cpx, vcenter),
			psy_ui_realsize_make(
				floor(tm->tmAveCharWidth / 2), barheight));
		r2 = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(r1.right + 2, vcenter),
			psy_ui_realsize_make(1, barheight));
		psy_ui_drawsolidrectangle(g, r1, psy_ui_component_colour(&self->component));		
		psy_ui_drawsolidrectangle(g, r2, psy_ui_component_colour(&self->component));
		psy_ui_textout(g, r2.right + 2, vcentertext, ":", 1);
	} else {
		psy_ui_RealRectangle r1;
		psy_ui_RealRectangle r2;

		r1 = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(cpx + 2, vcenter),
			psy_ui_realsize_make(
				floor(tm->tmAveCharWidth / 2), barheight));
		r2 = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(cpx, vcenter),
			psy_ui_realsize_make(1, barheight));

		psy_ui_drawsolidrectangle(g, r1, psy_ui_component_colour(&self->component));
		psy_ui_drawsolidrectangle(g, r2, psy_ui_component_colour(&self->component));
		psy_ui_textout(g, r2.left - tm->tmAveCharWidth, vcentertext, ":", 1);
	}
}

void seqeditloops_onmousedown(SeqEditLoops* self, psy_ui_MouseEvent* ev)
{
	const psy_ui_TextMetric* tm;
	psy_audio_Sequence* sequence;

	tm = psy_ui_component_textmetric(&self->component);
	sequence = seqeditstate_sequence(self->state);
	if (sequence && sequence->patterns) {
		psy_audio_Pattern* pattern;

		pattern = psy_audio_patterns_at(sequence->patterns,
			psy_audio_GLOBALPATTERN);
		if (pattern) {
			psy_audio_PatternNode* prev;
			psy_audio_PatternNode* node;
			double offset;
			psy_audio_PatternEntry* entry;
			int repeat;

			entry = NULL;			
			offset = seqeditstate_quantize(self->state,
				seqeditstate_pxtobeat(self->state, ev->pt.x));
			node = psy_audio_pattern_findnode(pattern, 1, offset - 1.0,
				1.0, &prev);
			repeat = 0;
			if (node) {
				entry = (psy_audio_PatternEntry*)node->entry;
				if (psy_audio_patternentry_front(entry)->cmd == 0xFE &&
					(psy_audio_patternentry_front(entry)->parameter & 0xF0) == 0xB0) {
					int repeat;

					repeat = psy_audio_patternentry_front(entry)->parameter & 0x0F;
				}
			}
			if (ev->button == 1) {
				if (entry) {
					self->drag = TRUE;
					if (repeat == 0) {
						self->nodeend = node;
						self->nodebegin = NULL;
						self->e = *psy_audio_patternentry_front(entry);
					} else {
						self->nodebegin = node;
						self->nodeend = seqeditloops_findloopend(self, node);						
					}
					psy_ui_component_capture(&self->component);
				}
			} else if (ev->button == 2) {
				if (entry) {					
					if (repeat == 0) {
						psy_audio_PatternNode* loopend;

						psy_audio_exclusivelock_enter();
						loopend = seqeditloops_findloopend(self, node);						
						psy_audio_pattern_remove(pattern, node);
						if (loopend) {
							psy_audio_pattern_remove(pattern, loopend);
						}
						psy_audio_sequencer_checkiterators(&self->state->workspace->player.sequencer, node);
						psy_audio_exclusivelock_leave();
						psy_signal_emit(&self->signal_changed, self, 0);
					} else {
						psy_audio_PatternNode* loopstart;

						loopstart = seqeditloops_findloopstart(self, node);
						psy_audio_exclusivelock_enter();
						psy_audio_pattern_remove(pattern, node);
						if (loopstart) {
							psy_audio_pattern_remove(pattern, loopstart);
						}
						psy_audio_sequencer_checkiterators(&self->state->workspace->player.sequencer, node);
						psy_audio_exclusivelock_leave();
						psy_signal_emit(&self->signal_changed, self, 0);
					}					
				}				
				psy_ui_component_invalidate(&self->component);
			}
		}
	}
}

void seqeditloops_onmousedoubleclick(SeqEditLoops* self, psy_ui_MouseEvent* ev)
{
	psy_audio_Sequence* sequence;

	sequence = seqeditstate_sequence(self->state);
	if (sequence && sequence->patterns) {
		psy_audio_Pattern* pattern;

		pattern = psy_audio_patterns_at(sequence->patterns,
			psy_audio_GLOBALPATTERN);
		if (pattern) {
			psy_audio_PatternEvent e;
			psy_audio_PatternNode* prev;
			psy_audio_PatternNode* node;
			psy_audio_exclusivelock_enter();
			node = psy_audio_pattern_findnode(pattern, 0 /* track 0 */,
				seqeditstate_quantize(self->state,
					seqeditstate_pxtobeat(self->state, ev->pt.x)),
				1.0, &prev);
			psy_audio_patternevent_init_all(&e, psy_audio_NOTECOMMANDS_EMPTY,
				0, 0, 0, 0xFE, 0xB0);
			psy_audio_pattern_insert(pattern, prev, 1,
				seqeditstate_quantize(self->state,
					seqeditstate_pxtobeat(self->state, ev->pt.x)), &e);
			node = psy_audio_pattern_findnode(pattern, 0 /* track 0 */,
				seqeditstate_quantize(self->state,
					seqeditstate_pxtobeat(self->state, ev->pt.x) + 4),
				1.0, &prev);
			psy_audio_patternevent_init_all(&e, psy_audio_NOTECOMMANDS_EMPTY,
				0, 0, 0, 0xFE, 0xB1);
			psy_audio_pattern_insert(pattern, prev, 1,
				seqeditstate_quantize(self->state,
					seqeditstate_pxtobeat(self->state, ev->pt.x)) + 4, &e);
			psy_audio_exclusivelock_leave();
			psy_ui_component_invalidate(&self->component);
			psy_signal_emit(&self->signal_changed, self, 0);
		}
	}
}

psy_audio_PatternNode* seqeditloops_findloopend(SeqEditLoops* self,
	psy_audio_PatternNode* begin)
{
	psy_audio_PatternNode* curr;
	psy_audio_PatternEntry* startentry;
	uintptr_t track;

	if (!begin) {
		return NULL;
	}
	startentry = (psy_audio_PatternEntry*)begin->entry;
	track = startentry->track;
	curr = begin->next;
	while (curr != NULL) {
		psy_audio_PatternEntry* entry;

		entry = (psy_audio_PatternEntry*)curr->entry;
		if (entry->track == track) {
			if (psy_audio_patternentry_front(entry)->cmd == 0xFE &&
				(psy_audio_patternentry_front(entry)->parameter & 0xF0) == 0xB0) {
				break;
			}
		}
		curr = curr->next;
	}
	return curr;
}

psy_audio_PatternNode* seqeditloops_findloopstart(SeqEditLoops* self,
	psy_audio_PatternNode* begin)
{
	psy_audio_PatternNode* curr;
	psy_audio_PatternEntry* startentry;
	uintptr_t track;

	if (!begin) {
		return NULL;
	}
	startentry = (psy_audio_PatternEntry*)begin->entry;
	track = startentry->track;
	curr = begin->prev;
	while (curr != NULL) {
		psy_audio_PatternEntry* entry;

		entry = (psy_audio_PatternEntry*)curr->entry;
		if (entry->track == track) {
			if (psy_audio_patternentry_front(entry)->cmd == 0xFE &&
				(psy_audio_patternentry_front(entry)->parameter & 0xF0) == 0xB0) {
				break;
			}
		}
		curr = curr->prev;
	}
	return curr;
}

void seqeditloops_onmousemove(SeqEditLoops* self, psy_ui_MouseEvent* ev)
{
	psy_dsp_big_beat_t position;

	position = seqeditstate_quantize(self->state,
		seqeditstate_pxtobeat(self->state, ev->pt.x));
	seqeditstate_setcursor(self->state, position);
	if (self->drag) {
		if (self->nodebegin == NULL && self->nodeend) {
			psy_audio_Sequence* sequence;

			sequence = seqeditstate_sequence(self->state);
			if (sequence && sequence->patterns) {
				psy_audio_Pattern* pattern;

				pattern = psy_audio_patterns_at(sequence->patterns,
					psy_audio_GLOBALPATTERN);
				if (pattern) {
					psy_audio_pattern_remove(pattern, self->nodeend);
					psy_audio_PatternNode* prev;
					psy_audio_PatternNode* node;
					psy_audio_exclusivelock_enter();
					node = psy_audio_pattern_findnode(pattern, 1, position, 1.0, &prev);
					self->nodeend = psy_audio_pattern_insert(pattern, prev, 1, position, &self->e);
					psy_audio_sequencer_checkiterators(&self->state->workspace->player.sequencer,
						self->nodeend);
					psy_audio_exclusivelock_leave();
					psy_ui_component_invalidate(&self->component);
					psy_signal_emit(&self->signal_changed, self, 0);
				}
			}
		}
	}
}

void seqeditloops_onmouseup(SeqEditLoops* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->component);
	self->drag = FALSE;
}

void seqeditloops_onmouseenter(SeqEditLoops* self)
{
	self->state->cursoractive = TRUE;
	psy_ui_component_invalidate(&self->component);
}

void seqeditloops_onmouseleave(SeqEditLoops* self)
{
	self->state->cursoractive = FALSE;
	psy_ui_component_invalidate(&self->component);
}

void seqeditloops_onpreferredsize(SeqEditLoops* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	if (seqeditstate_sequence(self->state)) {
		rv->width = psy_ui_value_make_px(
			seqeditstate_beattopx(self->state, psy_audio_sequence_duration(
				seqeditstate_sequence(self->state)) + 400.0));
	} else {
		rv->width = psy_ui_value_make_px(0.0);
	}
	rv->height = psy_ui_value_make_eh(2.0);
}
