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

static uint8_t numrepeat(psy_audio_PatternNode* node)
{	
	const psy_audio_PatternEntry* entry;
	
	entry = (const psy_audio_PatternEntry*)node->entry;
	if (entry) {
		 psy_audio_PatternEvent e;
		 
		 e = *psy_audio_patternentry_front((psy_audio_PatternEntry*)entry);
		 return (e.parameter & 0x0F);
	}
	return 0;					
}

static bool isloopstart(psy_audio_PatternNode* node)
{
	return numrepeat(node) == 0;
}


/* SeqEditLoopState */
void seqeditloopstate_init(SeqEditLoopState* self)
{
	seqeditloopstate_reset(self);
}

void seqeditloopstate_startdrag(SeqEditLoopState* self,
	psy_audio_PatternNode* node)
{	
	seqeditloopstate_reset(self);
	if (node) {
		if (isloopstart(node)) {
			self->start = node;
			self->end = self->next = psy_audio_patternnode_next_track(node,
				psy_audio_GLOBALPATTERN_LOOPTRACK);
			self->prev = psy_audio_patternnode_prev_track(node,
				psy_audio_GLOBALPATTERN_LOOPTRACK);
		} else {
			self->start = self->prev = psy_audio_patternnode_prev_track(node,
				psy_audio_GLOBALPATTERN_LOOPTRACK);
			self->end = node;
			self->next = psy_audio_patternnode_next_track(node,
				psy_audio_GLOBALPATTERN_LOOPTRACK);
		}		
		self->drag = TRUE;
	}
}

void seqeditloopstate_remove(SeqEditLoopState* self, psy_audio_PatternNode* node)
{
	seqeditloopstate_startdrag(self, node);
	self->drag = FALSE;
	self->remove = TRUE;
}

void seqeditloopstate_reset(SeqEditLoopState* self)
{
	self->drag = FALSE;
	self->remove = FALSE;
	self->start = self->end = self->prev = self->next = NULL;
}

/* SeqEditLoop*/
/* prototypes */
static void seqeditloop_ondraw(SeqEditLoop*, psy_ui_Graphics*);
static void seqeditloop_updatepattern(SeqEditLoop*);
static void seqeditloop_onmousedown(SeqEditLoop*, psy_ui_MouseEvent*);
static void seqeditloop_onmousemove(SeqEditLoop*, psy_ui_MouseEvent*);
static void seqeditloop_onmouseup(SeqEditLoop*, psy_ui_MouseEvent*);
static void seqeditloop_onmouseenter(SeqEditLoop*);
static void seqeditloop_onmouseleave(SeqEditLoop*);
static bool seqeditloop_boundsvalid(const SeqEditLoop*,
	psy_dsp_big_beat_t offset);
static void seqeditloop_select(SeqEditLoop*);
/* vtable */
static psy_ui_ComponentVtable seqeditloop_vtable;
static psy_ui_ComponentVtable seqeditloop_supervtable;
static bool seqeditloop_vtable_initialized = FALSE;

static void seqeditloop_vtable_init(SeqEditLoop* self)
{
	if (!seqeditloop_vtable_initialized) {
		seqeditloop_vtable = *(self->component.vtable);		
		seqeditloop_supervtable = seqeditloop_vtable;
		seqeditloop_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			seqeditloop_ondraw;
		seqeditloop_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditloop_onmousedown;
		seqeditloop_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqeditloop_onmousemove;
		seqeditloop_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			seqeditloop_onmouseup;
		seqeditloop_vtable.onmouseenter =
			(psy_ui_fp_component_event)
			seqeditloop_onmouseenter;
		seqeditloop_vtable.onmouseleave =
			(psy_ui_fp_component_event)
			seqeditloop_onmouseleave;
		seqeditloop_vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(&self->component, &seqeditloop_vtable);	
}
/* implementation */
void seqeditloop_init(SeqEditLoop* self, psy_ui_Component* parent,
	SeqEditLoopState* loopstate, SeqEditState* state,
	psy_audio_PatternNode* node)
{
	assert(self);
	assert(state);

	psy_ui_component_init(&self->component, parent, NULL);
	seqeditloop_vtable_init(self);
	self->state = state;
	self->loopstate = loopstate;
	self->node = node;	
	psy_ui_component_setstyletype(&self->component,
		STYLE_SEQEDT_LOOP);
	psy_ui_component_setstyletype_hover(&self->component,
		STYLE_SEQEDT_LOOP_HOVER);
	psy_ui_component_setstyletype_active(&self->component,
		STYLE_SEQEDT_LOOP_ACTIVE);		
	seqeditloop_updatepattern(self);
}

SeqEditLoop* seqeditloop_alloc(void)
{
	return (SeqEditLoop*)malloc(sizeof(SeqEditLoop));
}

SeqEditLoop* seqeditloop_allocinit(
	psy_ui_Component* parent, SeqEditLoopState* loopstate, SeqEditState* state,
	psy_audio_PatternNode* node)
{
	SeqEditLoop* rv;

	rv = seqeditloop_alloc();
	if (rv) {
		seqeditloop_init(rv, parent, loopstate, state, node);
		psy_ui_component_deallocateafterdestroyed(&rv->component);
	}
	return rv;
}

void seqeditloop_updatepattern(SeqEditLoop* self)
{
	psy_audio_Sequence* sequence;

	self->pattern = NULL;
	sequence = seqeditstate_sequence(self->state);
	if (sequence && sequence->patterns) {
		self->pattern = psy_audio_patterns_at(sequence->patterns,
			psy_audio_GLOBALPATTERN);
	} else {
		self->pattern = NULL;
	}	
}

void seqeditloop_ondraw(SeqEditLoop* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;
	double cpx;
	int repeat;
		
	tm = psy_ui_component_textmetric(&self->component);
	cpx = 0;
	size = psy_ui_component_size_px(&self->component);
	repeat = seqeditloop_repeat(self);	
	if (repeat == 0) {
		psy_ui_RealRectangle r1;
		psy_ui_RealRectangle r2;

		r1 = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(cpx, tm->tmHeight),
			psy_ui_realsize_make(
				floor(tm->tmAveCharWidth / 2), tm->tmHeight));
		r2 = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(r1.right + 2, tm->tmHeight),
			psy_ui_realsize_make(1, tm->tmHeight));
		psy_ui_drawsolidrectangle(g, r1, psy_ui_component_colour(&self->component));
		psy_ui_drawsolidrectangle(g, r2, psy_ui_component_colour(&self->component));
		psy_ui_textout(g, r2.right + 2, tm->tmHeight, ":", 1);
	} else {
		psy_ui_RealRectangle r1;
		psy_ui_RealRectangle r2;		

		r1 = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(cpx + 2, tm->tmHeight),
			psy_ui_realsize_make(
				floor(tm->tmAveCharWidth / 2), + tm->tmHeight));
		r2 = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(cpx, tm->tmHeight),
			psy_ui_realsize_make(1, + tm->tmHeight));
		psy_ui_drawsolidrectangle(g, r1, psy_ui_component_colour(&self->component));
		psy_ui_drawsolidrectangle(g, r2, psy_ui_component_colour(&self->component));
		psy_ui_textout(g, r2.left - tm->tmAveCharWidth, tm->tmHeight, ":", 1);
		if (repeat > 1) {
			char text[64];

			psy_snprintf(text, 64, "%dx", (int)repeat);
			psy_ui_textout(g, cpx - tm->tmAveCharWidth, 0, text, strlen(text));
		}
	}	
}

void seqeditloop_updateposition(SeqEditLoop* self)
{	
	psy_ui_component_setposition(&self->component,
		psy_ui_rectangle_make(
			psy_ui_point_make_px(
				seqeditstate_beattopx(self->state, seqeditloop_offset(self)),
				0.0),
			psy_ui_size_make(
				psy_ui_value_make_px(10.0),
				psy_ui_value_make_eh(2.0))));
}

void seqeditloop_onmousedown(SeqEditLoop* self, psy_ui_MouseEvent* ev)
{
	if (!self->pattern && !self->node) {
		psy_ui_mouseevent_stop_propagation(ev);
		return;
	}
	seqeditloopstate_startdrag(self->loopstate, self->node);
	if (psy_ui_mouseevent_button(ev) == 1) {
		seqeditloop_select(self);
		psy_ui_component_capture(&self->component);
	} else if (psy_ui_mouseevent_button(ev) == 2) {
		seqeditloopstate_remove(self->loopstate, self->node);		
	}
}

void seqeditloop_onmousemove(SeqEditLoop* self, psy_ui_MouseEvent* ev)
{
	psy_audio_PatternEvent e;	
	psy_dsp_big_beat_t offset;
	psy_ui_RealRectangle position;

	if (!self->loopstate->drag || !self->pattern || !seqeditloop_entry(self)) {
		return;
	}
	e = *psy_audio_patternentry_front(seqeditloop_entry(self));	
	position = psy_ui_component_position(&self->component);
	offset = seqeditstate_quantize(self->state,
		seqeditstate_pxtobeat(self->state, psy_ui_mouseevent_pt(ev).x + position.left));
	offset = psy_max(0.0, offset);
	if ((seqeditloop_offset(self) != offset) && seqeditloop_boundsvalid(self,
			offset)) {
		psy_audio_PatternNode* oldnode;
		psy_audio_PatternNode* node;
		psy_audio_PatternNode* prev;

		psy_audio_exclusivelock_enter();
		psy_audio_pattern_remove(self->pattern, self->node);
		node = psy_audio_pattern_findnode(self->pattern,
			psy_audio_GLOBALPATTERN_LOOPTRACK, offset, 1.0,
			&prev);
		if (!node) {
			node = prev;
		}
		oldnode = self->node;
		self->node = psy_audio_pattern_insert(self->pattern, prev,
			psy_audio_GLOBALPATTERN_LOOPTRACK,
			offset, &e);
		psy_audio_sequencer_checkiterators(
			&self->state->workspace->player.sequencer, oldnode);
		psy_audio_exclusivelock_leave();
		seqeditloop_updateposition(self);
		seqeditloop_select(self);
		psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
		seqeditstate_setcursor(self->state, offset);
	}		
	psy_ui_mouseevent_stop_propagation(ev);	
}

bool seqeditloop_boundsvalid(const SeqEditLoop* self,
	psy_dsp_big_beat_t offset)
{	
	return ((seqeditloop_repeat(self) == 0) &&
		(offset > seqeditloop_prevoffset(self) &&
		offset < seqeditloop_nextoffset(self))) ||
		((offset < seqeditloop_nextoffset(self) &&
		offset > seqeditloop_prevoffset(self)));		
}

void seqeditloop_onmouseup(SeqEditLoop* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_component_releasecapture(&self->component);
	seqeditloopstate_reset(self->loopstate);
	psy_ui_component_removestylestate(&self->component,
			psy_ui_STYLESTATE_ACTIVE);
}

void seqeditloop_onmouseenter(SeqEditLoop* self)
{
	seqeditloop_supervtable.onmouseenter(&self->component);
	self->state->cursoractive = TRUE;	
}

void seqeditloop_onmouseleave(SeqEditLoop* self)
{
	seqeditloop_supervtable.onmouseleave(&self->component);
	self->state->cursoractive = FALSE;	
}

void seqeditloop_select(SeqEditLoop* self)
{
	psy_ui_component_addstylestate(&self->component,
		psy_ui_STYLESTATE_ACTIVE);		
	psy_signal_emit(&self->state->signal_itemselected, self->state, 3,
		SEQEDITITEM_LOOP, 
		psy_audio_pattern_loop_index(self->pattern,
		self->node, psy_audio_GLOBALPATTERN_LOOPTRACK),
		psy_INDEX_INVALID);
}


/* SeqEditLoops*/
/* prototypes */
static void seqeditloops_ondestroy(SeqEditLoops*);
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
static void seqeditloops_onalign(SeqEditLoops*);
static void seqeditloops_onsongchanged(SeqEditLoops*, Workspace* sender);
static SeqEditLoop* seqeditloops_loopcomponent(SeqEditLoops*,
	psy_audio_PatternNode*);
static void seqeditloops_remove(SeqEditLoops*, psy_audio_PatternNode*);
static void seqeditloops_onloopchanged(SeqEditLoops*, SeqEditState* sender);
/* vtable */
static psy_ui_ComponentVtable seqeditloops_vtable;
static bool seqeditloops_vtable_initialized = FALSE;

static void seqeditloops_vtable_init(SeqEditLoops* self)
{
	if (!seqeditloops_vtable_initialized) {
		seqeditloops_vtable = *(self->component.vtable);
		seqeditloops_vtable.ondestroy =
			(psy_ui_fp_component_event)
			seqeditloops_ondestroy;
		seqeditloops_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqeditloops_onpreferredsize;
		seqeditloops_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditloops_onmousedown;
		seqeditloops_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqeditloops_onmousemove;
		seqeditloops_vtable.onmouseenter =
			(psy_ui_fp_component_event)
			seqeditloops_onmouseenter;
		seqeditloops_vtable.onmouseleave =
			(psy_ui_fp_component_event)
			seqeditloops_onmouseleave;
		seqeditloops_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			seqeditloops_onmousedoubleclick;
		seqeditloops_vtable.onalign =
			(psy_ui_fp_component_event)
			seqeditloops_onalign;
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
	assert(state->cmds);
	assert(state->cmds->workspace);

	psy_ui_component_init(&self->component, parent, NULL);	
	seqeditloops_vtable_init(self);		
	self->state = state;	
	self->entries = NULL;	
	seqeditloopstate_init(&self->loopstate);
	psy_ui_component_setpreferredheight(&self->component,
		psy_ui_value_make_eh(2.0));
	psy_signal_connect(&self->state->cmds->workspace->signal_songchanged, self,
		seqeditloops_onsongchanged);
	psy_signal_connect(&self->state->signal_loopchanged, self,
		seqeditloops_onloopchanged);
}

void seqeditloops_ondestroy(SeqEditLoops* self)
{	
	psy_list_free(self->entries);
	self->entries = NULL;
}

void seqeditloops_onmousedown(SeqEditLoops* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_button(ev) == 2 && self->loopstate.remove) {
		psy_signal_emit(&self->state->signal_itemselected, self->state, 3,
			SEQEDITITEM_LOOP, psy_INDEX_INVALID, psy_INDEX_INVALID);
		seqeditloops_remove(self, self->loopstate.start);
		seqeditloops_remove(self, self->loopstate.end);
		seqeditloopstate_reset(&self->loopstate);
		seqeditloops_onalign(self);		
		psy_ui_component_invalidate(&self->component);
	}		
}

void seqeditloops_remove(SeqEditLoops* self, psy_audio_PatternNode* node)
{
	SeqEditLoop* loop;

	loop = seqeditloops_loopcomponent(self, node);
	if (loop) {
		psy_List* p;

		p = psy_list_findentry(self->entries, loop);
		if (p) {
			psy_list_remove(&self->entries, p);
		}
		psy_audio_pattern_remove(loop->pattern, loop->node);
		psy_ui_component_remove(&self->component, &loop->component);
	}
}

SeqEditLoop* seqeditloops_loopcomponent(SeqEditLoops* self,
	psy_audio_PatternNode* node)
{
	SeqEditLoop* rv;
	psy_List* p;

	rv = NULL;
	for (p = self->entries; p != NULL; p = p->next) {
		SeqEditLoop* loop;
			
		loop = (SeqEditLoop*)(p->entry);
		if (loop->node == node) {
			rv = loop;
			break;
		}
	}	
	return rv;
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
			psy_audio_PatternNode* next;
			psy_audio_PatternEntry* nextentry;
			psy_audio_PatternNode* node;
			psy_dsp_big_beat_t insertlen;
			psy_dsp_big_beat_t insertoffset;
			psy_audio_exclusivelock_enter();
			
			/* insert loop start */
			insertoffset = seqeditstate_quantize(self->state,
				seqeditstate_pxtobeat(self->state,
					psy_ui_mouseevent_pt(ev).x));
			node = psy_audio_pattern_findnode(pattern,
				psy_audio_GLOBALPATTERN_LOOPTRACK,
				insertoffset, 1.0, &prev);
			if (!node) {
				node = prev;
			}
			if (node) {		
				psy_audio_PatternEntry* entry;
				psy_audio_PatternEvent e;
			
				entry = (psy_audio_PatternEntry*)(node->entry); 
				e = *psy_audio_patternentry_front(entry);
				if ((e.parameter & 0x0F) == 0) {
					workspace_outputstatus(self->state->cmds->workspace,
						"Can't insert inside a loop");
					return;					
				}
			}
			psy_audio_patternevent_init_all(&e, psy_audio_NOTECOMMANDS_EMPTY,
				0, 0, 0, 0xFE, 0xB0);
			node = psy_audio_pattern_insert(pattern, node, 
				psy_audio_GLOBALPATTERN_LOOPTRACK,
				insertoffset, &e);
			/* insert loop end */			
			next = psy_audio_patternnode_next_track(node,
				psy_audio_GLOBALPATTERN_LOOPTRACK);
			insertlen = 4.0;
			if (next) {
				nextentry = (psy_audio_PatternEntry*)next->entry;
				insertlen = psy_min(insertlen,
					nextentry->offset - insertoffset);

			}			
			psy_audio_patternevent_init_all(&e, psy_audio_NOTECOMMANDS_EMPTY,
				0, 0, 0, 0xFE, 0xB1);
			psy_audio_pattern_insert(pattern, node,
				psy_audio_GLOBALPATTERN_LOOPTRACK, insertoffset + insertlen, &e);
			psy_audio_exclusivelock_leave();
			psy_ui_component_invalidate(&self->component);
		}
	}
	seqeditloops_build(self);
}

void seqeditloops_onmousemove(SeqEditLoops* self, psy_ui_MouseEvent* ev)
{
	seqeditstate_setcursor(self->state, seqeditstate_quantize(self->state,
		seqeditstate_pxtobeat(self->state, psy_ui_mouseevent_pt(ev).x)));
}

void seqeditloops_onmouseenter(SeqEditLoops* self)
{
	self->state->cursoractive = TRUE;	
}

void seqeditloops_onmouseleave(SeqEditLoops* self)
{
	self->state->cursoractive = FALSE;
}

void seqeditloops_onpreferredsize(SeqEditLoops* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	rv->width = seqeditstate_preferredwidth(self->state);
}

void seqeditloops_build(SeqEditLoops* self)
{
	psy_audio_Sequence* sequence;

	psy_list_free(self->entries);
	self->entries = NULL;
	psy_ui_component_clear(&self->component);
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
							SeqEditLoop* loop;

							loop = seqeditloop_allocinit(&self->component,
								&self->loopstate, self->state, node);
							psy_list_append(&self->entries, loop);
						}
					}
				}
				seqeditloops_onalign(self);
			}
		}
	}
}

void seqeditloops_onalign(SeqEditLoops* self)
{
	psy_List* p;

	for (p = self->entries; p != NULL; p = p->next) {		
		seqeditloop_updateposition((SeqEditLoop*)(p->entry));
	}
}

void seqeditloops_onsongchanged(SeqEditLoops* self, Workspace* sender)
{
	seqeditloops_build(self);
	psy_ui_component_invalidate(&self->component);
}

void seqeditloops_onloopchanged(SeqEditLoops* self, SeqEditState* sender)
{
	psy_ui_component_invalidate(&self->component);
}