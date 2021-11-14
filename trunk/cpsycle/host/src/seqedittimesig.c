/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqedittimesig.h"
/* host */
#include "resources/resource.h"
#include "sequencetrackbox.h"
#include "styles.h"
/* audio */
#include <exclusivelock.h>
#include <sequencecmds.h>
/* platform */
#include "../../detail/portable.h"

#define DEFAULT_PXPERBEAT 10.0

/* SeqEditLoopState */
void seqedittimesigstate_init(SeqEditTimeSigState* self)
{
	seqedittimesigstate_reset(self);
}

void seqedittimesigstate_startdrag(SeqEditTimeSigState* self,
	psy_audio_PatternNode* node)
{	
	seqedittimesigstate_reset(self);
	if (node) {
		self->start = node;	
		self->drag = TRUE;
	}
}

void seqedittimesigstate_remove(SeqEditTimeSigState* self,
	psy_audio_PatternNode* node)
{
	seqedittimesigstate_startdrag(self, node);
	self->drag = FALSE;
	self->remove = TRUE;
}

void seqedittimesigstate_reset(SeqEditTimeSigState* self)
{
	self->drag = FALSE;
	self->remove = FALSE;
	self->start = NULL;
}

/* SeqEditTimeSig*/
/* prototypes */
static void seqedittimesig_updatepattern(SeqEditTimeSig*);
static void seqedittimesig_ondraw(SeqEditTimeSig*, psy_ui_Graphics*);
static void seqedittimesig_onmousedown(SeqEditTimeSig*, psy_ui_MouseEvent*);
static void seqedittimesig_onmousemove(SeqEditTimeSig*, psy_ui_MouseEvent*);
static void seqedittimesig_onmouseup(SeqEditTimeSig*, psy_ui_MouseEvent*);
static void seqedittimesig_onmouseenter(SeqEditTimeSig*);
static void seqedittimesig_onmouseleave(SeqEditTimeSig*);
/* vtable */
static psy_ui_ComponentVtable seqedittimesig_vtable;
static psy_ui_ComponentVtable seqedittimesig_supervtable;
static bool seqedittimesig_vtable_initialized = FALSE;

static void seqedittimesig_vtable_init(SeqEditTimeSig* self)
{
	if (!seqedittimesig_vtable_initialized) {
		seqedittimesig_vtable = *(self->component.vtable);		
		seqedittimesig_supervtable = seqedittimesig_vtable;
		seqedittimesig_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			seqedittimesig_ondraw;
		seqedittimesig_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqedittimesig_onmousedown;
		seqedittimesig_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqedittimesig_onmousemove;
		seqedittimesig_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			seqedittimesig_onmouseup;
		seqedittimesig_vtable.onmouseenter =
			(psy_ui_fp_component_event)
			seqedittimesig_onmouseenter;
		seqedittimesig_vtable.onmouseleave =
			(psy_ui_fp_component_event)
			seqedittimesig_onmouseleave;
		seqedittimesig_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqedittimesig_vtable;	
}
/* implementation */
void seqedittimesig_init(SeqEditTimeSig* self, psy_ui_Component* parent,
	psy_ui_Component* view, SeqEditTimeSigState* timesigstate, SeqEditState* state,
	psy_audio_PatternNode* node)
{
	assert(self);
	assert(state);

	psy_ui_component_init(&self->component, parent, view);
	seqedittimesig_vtable_init(self);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	psy_ui_component_setstyletype(&self->component,
		STYLE_SEQEDT_LOOP);
	psy_ui_component_setstyletype_hover(&self->component,
		STYLE_SEQEDT_LOOP_HOVER);
	psy_ui_component_setstyletype_active(&self->component,
		STYLE_SEQEDT_LOOP_ACTIVE);	
	self->state = state;
	self->timesigstate = timesigstate;
	self->node = node;	
	seqedittimesig_updatepattern(self);
}

SeqEditTimeSig* seqedittimesig_alloc(void)
{
	return (SeqEditTimeSig*)malloc(sizeof(SeqEditTimeSig));
}

SeqEditTimeSig* seqedittimesig_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditTimeSigState* timesigstate, SeqEditState* state,
	psy_audio_PatternNode* node)
{
	SeqEditTimeSig* rv;

	rv = seqedittimesig_alloc();
	if (rv) {
		seqedittimesig_init(rv, parent, view, timesigstate, state, node);
		psy_ui_component_deallocateafterdestroyed(&rv->component);
	}
	return rv;
}

void seqedittimesig_updatepattern(SeqEditTimeSig* self)
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

void seqedittimesig_updateposition(SeqEditTimeSig* self)
{	
	psy_ui_component_setposition(&self->component,
		psy_ui_rectangle_make(
			psy_ui_point_make_px(
				seqeditstate_beattopx(self->state,
					seqedittimesig_offset(self)), 0.0),
			psy_ui_size_make(
				psy_ui_value_make_px(10.0),
				psy_ui_value_make_eh(2.0))));
}

void seqedittimesig_ondraw(SeqEditTimeSig* self, psy_ui_Graphics* g)
{
	char text[64];
	const psy_ui_TextMetric* tm;
	double cpx;
	
	if (self->node) {
		psy_audio_PatternEntry* patternentry;
		psy_audio_PatternEvent* e;

		patternentry = (psy_audio_PatternEntry*)self->node->entry;
		e = psy_audio_patternentry_front(patternentry);
		if (e->note == psy_audio_NOTECOMMANDS_TIMESIG) {
			int nominator;
			int denominator;
			
			nominator = e->cmd;
			denominator = e->parameter;
			cpx = 0.0;
			tm = psy_ui_component_textmetric(&self->component);			
			psy_snprintf(text, 64, "%d", (int)(nominator));
			psy_ui_textout(g, cpx, 0, text, psy_strlen(text));
			psy_snprintf(text, 64, "%d", (int)(denominator));
			psy_ui_textout(g, cpx, tm->tmHeight, text, psy_strlen(text));
		}
	}
}

void seqedittimesig_onmousedown(SeqEditTimeSig* self, psy_ui_MouseEvent* ev)
{
	if (!self->pattern && !self->node) {
		psy_ui_mouseevent_stop_propagation(ev);
		return;
	}	
	seqedittimesigstate_startdrag(self->timesigstate, self->node);	
	if (ev->button == 1) {
		seqedittimesig_select(self);
		psy_ui_component_capture(&self->component);		
	} else if (ev->button == 2) {
		seqedittimesigstate_remove(self->timesigstate, self->node);		
	}
}

void seqedittimesig_onmousemove(SeqEditTimeSig* self, psy_ui_MouseEvent* ev)
{
	psy_audio_PatternEvent e;	
	psy_dsp_big_beat_t offset;
	psy_ui_RealRectangle position;

	if (!self->timesigstate->drag || !self->pattern ||
			!seqedittimesig_entry_const(self)) {
		return;
	}
	e = *psy_audio_patternentry_front(seqedittimesig_entry(self));	
	position = psy_ui_component_position(&self->component);
	offset = seqeditstate_quantize(self->state,
		seqeditstate_pxtobeat(self->state, ev->pt.x + position.left));
	offset = psy_max(0.0, offset);
	if ((seqedittimesig_offset(self) != offset)) {
		psy_audio_PatternNode* oldnode;
		psy_audio_PatternNode* node;
		psy_audio_PatternNode* prev;

		psy_audio_exclusivelock_enter();
		psy_audio_pattern_remove(self->pattern, self->node);
		node = psy_audio_pattern_findnode(self->pattern,
			psy_audio_GLOBALPATTERN_TIMESIGTRACK, offset, 1.0,
			&prev);
		if (!node) {
			node = prev;
		}
		oldnode = self->node;
		self->node = psy_audio_pattern_insert(self->pattern, prev,
			psy_audio_GLOBALPATTERN_TIMESIGTRACK,
			offset, &e);
		psy_audio_sequencer_checkiterators(
			&self->state->workspace->player.sequencer, oldnode);
		psy_audio_exclusivelock_leave();
		seqedittimesig_updateposition(self);
		psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
		seqeditstate_setcursor(self->state, offset);
		seqedittimesig_select(self);
	}		
	psy_ui_mouseevent_stop_propagation(ev);	
}

void seqedittimesig_onmouseup(SeqEditTimeSig* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->component);
	seqedittimesigstate_reset(self->timesigstate);
	psy_ui_component_removestylestate(&self->component,
		psy_ui_STYLESTATE_ACTIVE);
}

void seqedittimesig_select(SeqEditTimeSig* self)
{
	psy_ui_component_addstylestate(&self->component,
		psy_ui_STYLESTATE_ACTIVE);		
	psy_signal_emit(&self->state->signal_itemselected, self->state, 3,
		SEQEDITITEM_TIMESIG, 
		psy_audio_pattern_timesig_index(self->pattern,
		self->node, psy_audio_GLOBALPATTERN_TIMESIGTRACK),
		psy_INDEX_INVALID);
}

void seqedittimesig_onmouseenter(SeqEditTimeSig* self)
{
	seqedittimesig_supervtable.onmouseenter(&self->component);
	self->state->cursoractive = TRUE;	
}

void seqedittimesig_onmouseleave(SeqEditTimeSig* self)
{
	seqedittimesig_supervtable.onmouseleave(&self->component);
	self->state->cursoractive = FALSE;	
}


/* SeqEditTimeSig*/
/* prototypes */
static void seqedittimesigs_ondestroy(SeqEditTimeSigs*);
static void seqedittimesigs_onmousedown(SeqEditTimeSigs*, psy_ui_MouseEvent*);
static void seqedittimesigs_onmousemove(SeqEditTimeSigs*, psy_ui_MouseEvent*);
static void seqedittimesigs_onmouseenter(SeqEditTimeSigs*);
static void seqedittimesigs_onmouseleave(SeqEditTimeSigs*);
static void seqedittimesigs_onmousedoubleclick(SeqEditTimeSigs*, psy_ui_MouseEvent*);
static void seqedittimesigs_onpreferredsize(SeqEditTimeSigs*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqedittimesigs_editnode(SeqEditTimeSigs*, psy_audio_PatternNode*,
	psy_ui_RealPoint);
static void seqedittimesigs_onalign(SeqEditTimeSigs*);
static void seqedittimesigs_onsongchanged(SeqEditTimeSigs*, Workspace*, int flag);
static void seqedittimesigs_build(SeqEditTimeSigs*);
static void seqedittimesigs_ontimesigchanged(SeqEditTimeSigs*, SeqEditState* sender);
static void seqedittimesigs_remove(SeqEditTimeSigs*, psy_audio_PatternNode*);
static SeqEditTimeSig* seqedittimesigs_timesigcomponent(SeqEditTimeSigs*,
	psy_audio_PatternNode*);
/* vtable */
static psy_ui_ComponentVtable seqedittimesigs_vtable;
static bool seqedittimesigs_vtable_initialized = FALSE;

static void seqedittimesigs_vtable_init(SeqEditTimeSigs* self)
{
	if (!seqedittimesigs_vtable_initialized) {
		seqedittimesigs_vtable = *(self->component.vtable);
		seqedittimesigs_vtable.ondestroy =
			(psy_ui_fp_component_event)
			seqedittimesigs_ondestroy;		
		seqedittimesigs_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqedittimesigs_onpreferredsize;
		seqedittimesigs_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqedittimesigs_onmousedown;	
		seqedittimesigs_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqedittimesigs_onmousemove;
		seqedittimesigs_vtable.onmouseenter =
			(psy_ui_fp_component_event)
			seqedittimesigs_onmouseenter;
		seqedittimesigs_vtable.onmouseleave =
			(psy_ui_fp_component_event)
			seqedittimesigs_onmouseleave;
		seqedittimesigs_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			seqedittimesigs_onmousedoubleclick;
		seqedittimesigs_vtable.onalign =
			(psy_ui_fp_component_event)
			seqedittimesigs_onalign;
		seqedittimesigs_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqedittimesigs_vtable;
}
void seqedittimesigs_init(SeqEditTimeSigs* self, psy_ui_Component* parent,
	SeqEditState* state)
{
	assert(self);
	assert(state);

	psy_ui_component_init(&self->component, parent, parent);
	seqedittimesigs_vtable_init(self);		
	self->state = state;		
	self->entries = NULL;
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_NOBACKGROUND);
	seqedittimesigstate_init(&self->timesigstate);
	psy_signal_connect(&self->state->cmds->workspace->signal_songchanged, self,
		seqedittimesigs_onsongchanged);
	seqedittimesigs_build(self);
	psy_signal_connect(&self->state->signal_timesigchanged, self,
		seqedittimesigs_ontimesigchanged);
}

void seqedittimesigs_ondestroy(SeqEditTimeSigs* self)
{	
	psy_list_free(self->entries);
	self->entries = NULL;
}

void seqedittimesigs_onmousedown(SeqEditTimeSigs* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 2 && self->timesigstate.remove) {
		psy_signal_emit(&self->state->signal_itemselected, self->state, 3,
			SEQEDITITEM_TIMESIG, psy_INDEX_INVALID, psy_INDEX_INVALID);
		seqedittimesigs_remove(self, self->timesigstate.start);
		seqedittimesigstate_reset(&self->timesigstate);
		seqedittimesigs_onalign(self);
		psy_ui_component_invalidate(&self->component);		
	}	
}

void seqedittimesigs_remove(SeqEditTimeSigs* self, psy_audio_PatternNode* node)
{
	SeqEditTimeSig* timesig;

	timesig = seqedittimesigs_timesigcomponent(self, node);
	if (timesig) {
		psy_List* p;

		p = psy_list_findentry(self->entries, timesig);
		if (p) {
			psy_list_remove(&self->entries, p);
		}
		psy_audio_pattern_remove(timesig->pattern, timesig->node);
		psy_ui_component_remove(&self->component, &timesig->component);
	}
}

SeqEditTimeSig* seqedittimesigs_timesigcomponent(SeqEditTimeSigs* self,
	psy_audio_PatternNode* node)
{
	SeqEditTimeSig* rv;
	psy_List* p;

	rv = NULL;
	for (p = self->entries; p != NULL; p = p->next) {
		SeqEditTimeSig* loop;
			
		loop = (SeqEditTimeSig*)(p->entry);
		if (loop->node == node) {
			rv = loop;
			break;
		}
	}	
	return rv;
}

void seqedittimesigs_onmousedoubleclick(SeqEditTimeSigs* self,
	psy_ui_MouseEvent* ev)
{	
	SeqEditTimeSig* seqedittimesig;
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* prev;
	psy_audio_PatternEvent e;

	node = seqeditstate_node(self->state, ev->pt, 0.0, 1.0, &prev);			
	psy_audio_patternevent_init_all(&e,
		psy_audio_NOTECOMMANDS_TIMESIG, 0, 0, 0, 4, 4);
	node = psy_audio_pattern_insert(seqeditstate_globalpattern(self->state), prev,
		0, seqeditstate_quantize(self->state,
			seqeditstate_pxtobeat(self->state, ev->pt.x)), &e);
	seqedittimesigs_build(self);
	seqedittimesig = seqedittimesigs_timesigcomponent(self, node);
	if (seqedittimesig) {
		seqedittimesig_select(seqedittimesig);
	}
	psy_ui_component_invalidate(&self->component);
}

void seqedittimesigs_onmousemove(SeqEditTimeSigs* self, psy_ui_MouseEvent* ev)
{
	seqeditstate_setcursor(self->state, seqeditstate_quantize(self->state,
		seqeditstate_pxtobeat(self->state, ev->pt.x)));
}

void seqedittimesigs_onmouseenter(SeqEditTimeSigs* self)
{
	self->state->cursoractive = TRUE;
}

void seqedittimesigs_onmouseleave(SeqEditTimeSigs* self)
{
	self->state->cursoractive = FALSE;	
}

void seqedittimesigs_onpreferredsize(SeqEditTimeSigs* self,
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

void seqedittimesigs_build(SeqEditTimeSigs* self)
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
						if (e->note == psy_audio_NOTECOMMANDS_TIMESIG) {
							SeqEditTimeSig* timesig;

							timesig = seqedittimesig_allocinit(&self->component,
								NULL, &self->timesigstate, self->state, node);
							psy_list_append(&self->entries, timesig);
						}
					}
				}
				seqedittimesigs_onalign(self);
			}
		}
	}
}

void seqedittimesigs_onalign(SeqEditTimeSigs* self)
{
	psy_List* p;

	for (p = self->entries; p != NULL; p = p->next) {		
		seqedittimesig_updateposition((SeqEditTimeSig*)(p->entry));
	}
}

void seqedittimesigs_onsongchanged(SeqEditTimeSigs* self, Workspace* sender,
	int flag)
{
	seqedittimesigs_build(self);
	psy_ui_component_invalidate(&self->component);
}

void seqedittimesigs_ontimesigchanged(SeqEditTimeSigs* self, SeqEditState* sender)
{
	psy_ui_component_invalidate(&self->component);
}
