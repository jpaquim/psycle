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

/* SeqEditTimeSig*/
/* prototypes */
static void seqedittimesig_ondraw(SeqEditTimeSig*, psy_ui_Graphics*);
static void seqedittimesig_drawtimesigs(SeqEditTimeSig*, psy_ui_Graphics*);
static void seqedittimesig_drawtimesig(SeqEditTimeSig*, psy_ui_Graphics*,
	psy_dsp_big_beat_t offset, int nominator, int denominator);
static void seqedittimesig_oneditaccept(SeqEditTimeSig*, psy_ui_Edit* sender);
static void seqedittimesig_oneditreject(SeqEditTimeSig*, psy_ui_Edit* sender);
static void seqedittimesig_onmousedown(SeqEditTimeSig*, psy_ui_MouseEvent*);
static void seqedittimesig_onmousemove(SeqEditTimeSig*, psy_ui_MouseEvent*);
static void seqedittimesig_onmouseenter(SeqEditTimeSig*);
static void seqedittimesig_onmouseleave(SeqEditTimeSig*);
static void seqedittimesig_onmousedoubleclick(SeqEditTimeSig*, psy_ui_MouseEvent*);
static void seqedittimesig_onpreferredsize(SeqEditTimeSig*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqedittimesig_editnode(SeqEditTimeSig*, psy_audio_PatternNode*,
	psy_ui_RealPoint);
/* vtable */
static psy_ui_ComponentVtable seqedittimesig_vtable;
static bool seqedittimesig_vtable_initialized = FALSE;

static void seqedittimesig_vtable_init(SeqEditTimeSig* self)
{
	if (!seqedittimesig_vtable_initialized) {
		seqedittimesig_vtable = *(self->component.vtable);
		seqedittimesig_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			seqedittimesig_ondraw;
		seqedittimesig_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqedittimesig_onpreferredsize;
		seqedittimesig_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqedittimesig_onmousedown;
		seqedittimesig_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqedittimesig_onmousemove;
		seqedittimesig_vtable.onmouseenter =
			(psy_ui_fp_component_onmouseenter)
			seqedittimesig_onmouseenter;
		seqedittimesig_vtable.onmouseleave =
			(psy_ui_fp_component_onmouseleave)
			seqedittimesig_onmouseleave;
		seqedittimesig_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			seqedittimesig_onmousedoubleclick;
		seqedittimesig_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqedittimesig_vtable;
}
void seqedittimesig_init(SeqEditTimeSig* self, psy_ui_Component* parent,
	SeqEditState* state)
{
	assert(self);
	assert(state);

	psy_ui_component_init(&self->component, parent, parent);
	seqedittimesig_vtable_init(self);
	psy_ui_component_doublebuffer(&self->component);
	self->state = state;		
	self->preventedit = TRUE;
	self->editnominator = TRUE;
	self->currtimesig = NULL;	
	self->preventedit = TRUE;
	if (self->state->edit) {
		psy_signal_connect(&self->state->edit->signal_accept,
			self, seqedittimesig_oneditaccept);
		psy_signal_connect(&self->state->edit->signal_reject,
			self, seqedittimesig_oneditreject);
	}
}

void seqedittimesig_oneditaccept(SeqEditTimeSig* self, psy_ui_Edit* sender)
{
	if (!self->preventedit) {
		self->preventedit = TRUE;
		if (self->editnominator && self->currtimesig) {
			int nominator;

			nominator = atoi(psy_ui_edit_text(self->state->edit));
			self->currtimesig->cmd = (uint8_t)nominator;
		} else if (self->currtimesig) {
			int denominator;

			denominator = atoi(psy_ui_edit_text(self->state->edit));
			self->currtimesig->parameter = (uint8_t)denominator;
		}
		self->currtimesig = NULL;
		psy_ui_component_hide(psy_ui_edit_base(sender));
		psy_ui_component_setparent(&self->state->edit->component,
			self->state->view);
		psy_ui_component_invalidate(&self->component);
	}
}

void seqedittimesig_oneditreject(SeqEditTimeSig* self, psy_ui_Edit* sender)
{
	if (!self->preventedit) {
		self->preventedit = TRUE;
		self->currtimesig = NULL;
		psy_ui_component_hide(psy_ui_edit_base(sender));
		psy_ui_component_setparent(&self->state->edit->component,
			self->state->view);
	}
}

void seqedittimesig_ondraw(SeqEditTimeSig* self, psy_ui_Graphics* g)
{		
	seqedittimesig_drawtimesigs(self, g);	
}

void seqedittimesig_drawtimesigs(SeqEditTimeSig* self, psy_ui_Graphics* g)
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
						if (e->note == psy_audio_NOTECOMMANDS_TIMESIG) {
							seqedittimesig_drawtimesig(self, g, patternentry->offset, e->cmd,
								e->parameter);
						}
					}
				}
			}
		}
	}
}

void seqedittimesig_drawtimesig(SeqEditTimeSig* self, psy_ui_Graphics* g,
	psy_dsp_big_beat_t offset, int nominator, int denominator)
{
	char text[64];
	const psy_ui_TextMetric* tm;
	double cpx;

	tm = psy_ui_component_textmetric(&self->component);
	cpx = seqeditstate_beattopx(self->state, offset);
	psy_snprintf(text, 64, "%d", (int)(nominator));
	psy_ui_textout(g, cpx, 0, text, psy_strlen(text));
	psy_snprintf(text, 64, "%d", (int)(denominator));
	psy_ui_textout(g, cpx, tm->tmHeight, text, psy_strlen(text));
}


void seqedittimesig_onmousedown(SeqEditTimeSig* self, psy_ui_MouseEvent* ev)
{
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* prev;
	
	node = seqeditstate_node(self->state, ev->pt, -1.0, 1.0, &prev);
	if (node) {
		if (ev->button == 1) {
			seqedittimesig_editnode(self, node, ev->pt);
		} else if (ev->button == 2) {					
			psy_audio_pattern_remove(seqeditstate_globalpattern(self->state),
				node);
			self->currtimesig = NULL;
			psy_ui_component_invalidate(&self->component);
		}
	} else {
		self->currtimesig = NULL;
	}	
}

void seqedittimesig_onmousedoubleclick(SeqEditTimeSig* self, psy_ui_MouseEvent* ev)
{	
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* prev;
	psy_audio_PatternEvent e;

	node = seqeditstate_node(self->state, ev->pt, -1.0, 1.0, &prev);			
	psy_audio_patternevent_init_all(&e,
		psy_audio_NOTECOMMANDS_TIMESIG, 0, 0, 0, 4, 4);
	psy_audio_pattern_insert(seqeditstate_globalpattern(self->state), prev,
		0, seqeditstate_quantize(self->state,
			seqeditstate_pxtobeat(self->state, ev->pt.x)), &e);
	psy_ui_component_invalidate(&self->component);	
}

void seqedittimesig_editnode(SeqEditTimeSig* self, psy_audio_PatternNode* node,
	psy_ui_RealPoint pt)
{
	if (node) {
		char text[64];
		psy_audio_PatternEntry* entry;
		psy_audio_PatternEvent* e;
		const psy_ui_TextMetric* tm;
		psy_ui_RealPoint cp;

		tm = psy_ui_component_textmetric(&self->component);
		entry = (psy_audio_PatternEntry*)node->entry;
		e = psy_audio_patternentry_front(entry);
		self->currtimesig = e;
		cp.x = seqeditstate_beattopx(self->state, entry->offset) - 4;
		if (pt.y < tm->tmHeight) {
			psy_snprintf(text, 64, "%d", (int)e->cmd);
			self->editnominator = TRUE;
			cp.y = 0.0;
		} else {
			psy_snprintf(text, 64, "%d", (int)e->parameter);
			self->editnominator = FALSE;
			cp.y = tm->tmHeight;
		}
		seqeditstate_edit(self->state, &self->component,
			cp, 4.0, text);
		self->preventedit = FALSE;
	}
}

void seqedittimesig_onmousemove(SeqEditTimeSig* self, psy_ui_MouseEvent* ev)
{
	seqeditstate_setcursor(self->state, seqeditstate_quantize(self->state,
		seqeditstate_pxtobeat(self->state, ev->pt.x)));
}

void seqedittimesig_onmouseenter(SeqEditTimeSig* self)
{
	self->state->cursoractive = TRUE;
	psy_ui_component_invalidate(&self->component);
}

void seqedittimesig_onmouseleave(SeqEditTimeSig* self)
{
	self->state->cursoractive = FALSE;
	psy_ui_component_invalidate(&self->component);
}

void seqedittimesig_onpreferredsize(SeqEditTimeSig* self,
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
