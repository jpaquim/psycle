/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditproperties.h"
/* host */
#include "sequencetrackbox.h"
#include "styles.h"
/* platform */
#include "../../detail/portable.h"

/* SeqEditEntryProperties */
/* prototypes */
static void seqeditentryproperties_update(SeqEditEntryProperties*);

/* implementation */
void seqeditentryproperties_init(SeqEditEntryProperties* self, psy_ui_Component* parent,
	SeqEditState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	self->state = state;
	self->orderindex = psy_audio_orderindex_zero();
	/* position */
	labelpair_init(&self->offset, &self->component, "Position", 12.0);
	psy_ui_component_setalign(psy_ui_label_base(&self->offset.second),
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(labelpair_base(&self->offset), psy_ui_ALIGN_TOP);
	/* length */
	labelpair_init(&self->length, &self->component, "Length", 12.0);
	psy_ui_component_setalign(psy_ui_label_base(&self->length.second),
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(labelpair_base(&self->length), psy_ui_ALIGN_TOP);
	/* end */
	labelpair_init(&self->end, &self->component, "End", 12.0);
	psy_ui_component_setalign(psy_ui_label_base(&self->end.second),
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(labelpair_base(&self->end), psy_ui_ALIGN_TOP);	
}

void seqeditentryproperties_setorderindex(SeqEditEntryProperties* self,
	psy_audio_OrderIndex orderindex)
{
	self->orderindex = orderindex;
	seqeditentryproperties_update(self);
}

void seqeditentryproperties_update(SeqEditEntryProperties* self)
{
	psy_audio_Sequence* sequence;

	sequence = seqeditstate_sequence(self->state);
	if (sequence) {
		char text[64];
		psy_audio_SequenceEntry* seqentry;

		seqentry = psy_audio_sequence_entry(sequence, self->orderindex);
		*text = '\0';
		if (seqentry) {
			psy_snprintf(text, 64, "%7.3f", (float)psy_audio_sequenceentry_offset(seqentry));
		}
		psy_ui_label_settext(&self->offset.second, text);
		*text = '\0';
		if (seqentry) {
			psy_snprintf(text, 64, "%7.3f", (float)psy_audio_sequenceentry_length(seqentry));
		}
		psy_ui_label_settext(&self->length.second, text);
		*text = '\0';
		if (seqentry) {
			psy_snprintf(text, 64, "%7.3f", (float)(psy_audio_sequenceentry_offset(seqentry) +
				psy_audio_sequenceentry_length(seqentry)));
		}
		psy_ui_label_settext(&self->end.second, text);
	}
}

/* SeqEditLoopProperties */

/* implementation */
void seqeditemptyproperties_init(SeqEditEmptyProperties* self, psy_ui_Component* parent,
	SeqEditState* state)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	self->state = state;		
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(40.0, 20.0));
	psy_ui_component_setspacing(&self->component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));
	psy_ui_label_init(&self->caption, &self->component);
	psy_ui_label_settext(&self->caption, "No Item selected");
	psy_ui_component_setalign(psy_ui_label_base(&self->caption),
		psy_ui_ALIGN_TOP);
}

/* SeqEditPatternProperties */

/* implementation */
void seqeditpatternproperties_init(SeqEditPatternProperties* self, psy_ui_Component* parent,
	SeqEditState* state)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	self->state = state;		
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(40.0, 20.0));
	psy_ui_component_setspacing(&self->component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));
	psy_ui_label_init(&self->caption, &self->component);
	psy_ui_label_settext(&self->caption, "Pattern Item selected");
	psy_ui_component_setalign(psy_ui_label_base(&self->caption),
		psy_ui_ALIGN_TOP);
	/* seqentry properties */
	seqeditentryproperties_init(&self->entry, &self->component, self->state);
	psy_ui_component_setalign(&self->entry.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->entry.component,
		psy_ui_margin_make_em(0.5, 0.0, 0.0, 0.0));
}

void seqeditpatternproperties_setorderindex(SeqEditPatternProperties* self,
	psy_audio_OrderIndex orderindex)
{		
	seqeditentryproperties_setorderindex(&self->entry, orderindex);	
}

/* SeqEditSampleProperties */

/* implementation */
void seqeditsampleproperties_init(SeqEditSampleProperties* self, psy_ui_Component* parent,
	SeqEditState* state)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	self->state = state;		
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(40.0, 20.0));
	psy_ui_component_setspacing(&self->component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));
	psy_ui_label_init(&self->caption, &self->component);
	psy_ui_label_settext(&self->caption, "Sample Item selected");
	psy_ui_component_setalign(psy_ui_label_base(&self->caption),
		psy_ui_ALIGN_TOP);
	/* seqentry properties */
	seqeditentryproperties_init(&self->entry, &self->component, self->state);
	psy_ui_component_setalign(&self->entry.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->entry.component,
		psy_ui_margin_make_em(0.5, 0.0, 0.0, 0.0));
}

void seqeditsampleproperties_setorderindex(SeqEditSampleProperties* self,
	psy_audio_OrderIndex orderindex)
{		
	seqeditentryproperties_setorderindex(&self->entry, orderindex);	
}

/* SeqEditMarkerProperties */

/* prototypes */
static void seqeditmarkerproperties_oneditaccept(SeqEditMarkerProperties*,
	psy_ui_TextInput* sender);
static void seqeditmarkerproperties_oneditreject(SeqEditMarkerProperties*,
	psy_ui_TextInput* sender);

/* implementation */
void seqeditmarkerproperties_init(SeqEditMarkerProperties* self, psy_ui_Component* parent,
	SeqEditState* state)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	self->state = state;			
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(40.0, 20.0));
	psy_ui_component_setspacing(&self->component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));
	psy_ui_label_init(&self->caption, &self->component);
	psy_ui_label_settext(&self->caption, "Marker Item selected");
	psy_ui_component_setalign(psy_ui_label_base(&self->caption),
		psy_ui_ALIGN_TOP);
	labeledit_init(&self->name, &self->component, "Name");
	psy_ui_component_setmargin(&self->name.component,
		psy_ui_margin_make_em(0.5, 0.0, 0.0, 0.0));	
	psy_ui_component_setalign(labeledit_base(&self->name),
		psy_ui_ALIGN_TOP);
	/* seqentry properties */
	seqeditentryproperties_init(&self->entry, &self->component, self->state);
	psy_ui_component_setalign(&self->entry.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->entry.component,
		psy_ui_margin_make_em(0.5, 0.0, 0.0, 0.0));
	/* connect */
	psy_signal_connect(&self->name.edit.signal_accept,
		self, seqeditmarkerproperties_oneditaccept);
	psy_signal_connect(&self->name.edit.signal_reject,
		self, seqeditmarkerproperties_oneditreject);	
}

void seqeditmarkerproperties_setorderindex(SeqEditMarkerProperties* self,
	psy_audio_OrderIndex orderindex)
{
	psy_audio_SequenceEntry* entry;	
		
	seqeditentryproperties_setorderindex(&self->entry, orderindex);
	entry = psy_audio_sequence_entry(seqeditstate_sequence(self->state),
		orderindex);
	if (entry && entry->type == psy_audio_SEQUENCEENTRY_MARKER) {		
		psy_audio_SequenceMarkerEntry* markerentry;

		markerentry = (psy_audio_SequenceMarkerEntry*)entry;		
		psy_ui_textinput_settext(&self->name.edit, markerentry->text);		
	}
}

void seqeditmarkerproperties_oneditaccept(SeqEditMarkerProperties* self,
	psy_ui_TextInput* sender)
{
	psy_audio_Sequence* sequence;
	
	sequence = seqeditstate_sequence(self->state);
	if (sequence) {
		psy_audio_SequenceEntry* seqentry;

		seqentry = psy_audio_sequence_entry(sequence, self->entry.orderindex);
		if (seqentry && seqentry->type == psy_audio_SEQUENCEENTRY_MARKER) {
			psy_audio_SequenceMarkerEntry* markerentry;

			markerentry = (psy_audio_SequenceMarkerEntry*)seqentry;
			psy_strreset(&markerentry->text, psy_ui_textinput_text(sender));
			psy_ui_component_invalidate(self->state->view);
			psy_ui_component_setfocus(self->state->view);
		}
	}
}

void seqeditmarkerproperties_oneditreject(SeqEditMarkerProperties* self,
	psy_ui_TextInput* sender)
{
	psy_ui_component_setfocus(self->state->view);
}

/* SeqEditTimesigProperties */
/* prototypes */
void seqedittimesigproperties_onnominator(SeqEditTimesigProperties*, IntEdit* sender);
void seqedittimesigproperties_ondenominator(SeqEditTimesigProperties*, IntEdit* sender);

/* implementation */
void seqedittimesigproperties_init(SeqEditTimesigProperties* self, psy_ui_Component* parent,
	SeqEditState* state)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	self->state = state;
	self->timesigindex = psy_INDEX_INVALID;		
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(40.0, 20.0));
	psy_ui_component_setspacing(&self->component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));
	psy_ui_label_init(&self->caption, &self->component);
	psy_ui_label_settext(&self->caption, "Timesig Item selected");
	psy_ui_component_setalign(psy_ui_label_base(&self->caption),
		psy_ui_ALIGN_TOP);
	/* nominator */
	intedit_init(&self->nominator, &self->component, "Nominator", 1, 1, 0xF);
	psy_ui_component_setalign(intedit_base(&self->nominator),
		psy_ui_ALIGN_TOP);
	psy_ui_label_setcharnumber(&self->nominator.desc, 14.0);
	psy_ui_component_setmargin(&self->nominator.component,
		psy_ui_margin_make_em(0.5, 0.0, 0.0, 0.0));	
	/* denominator */
	intedit_init(&self->denominator, &self->component, "Denominator", 1, 1, 0xF);
	psy_ui_component_setalign(intedit_base(&self->denominator),
		psy_ui_ALIGN_TOP);
	psy_ui_label_setcharnumber(&self->denominator.desc, 14.0);
	/* position */	
	labelpair_init(&self->offset, &self->component, "Position", 12.0);
	psy_ui_component_setalign(labelpair_base(&self->offset), psy_ui_ALIGN_TOP);	
	psy_ui_component_setmargin(labelpair_base(&self->offset), psy_ui_margin_make_em(
		0.5, 0.0, 0.0, 0.0));
	/* connect signals */
	psy_signal_connect(&self->nominator.signal_changed, self,
		seqedittimesigproperties_onnominator);	
	psy_signal_connect(&self->denominator.signal_changed, self,
		seqedittimesigproperties_ondenominator);	
}

void seqedittimesigproperties_settimesigindex(SeqEditTimesigProperties* self,
	uintptr_t timesigindex)
{
	psy_audio_Pattern* pattern;

	self->timesigindex = timesigindex;	
	pattern = psy_audio_patterns_at(
		&self->state->cmds->workspace->song->patterns,
		psy_audio_GLOBALPATTERN);			
	if (timesigindex == psy_INDEX_INVALID) {						
		psy_ui_label_settext(&self->caption, "No Timesig selected");	
		return;
	} else {
		psy_ui_label_settext(&self->caption, "TimeSig Item selected");	
	}
	if (pattern) {
		psy_audio_PatternNode* node;
		psy_audio_PatternEntry* entry;
		psy_audio_PatternEvent e;
					
		node = psy_audio_pattern_timesig_at(pattern, timesigindex,
			psy_audio_GLOBALPATTERN_TIMESIGTRACK);
		if (node) {
			char text[64];

			entry = (psy_audio_PatternEntry*)(node->entry); 
			e = *psy_audio_patternentry_front(entry);
			intedit_setvalue(&self->nominator, (e.cmd));
			intedit_setvalue(&self->denominator, (e.parameter));				
			psy_snprintf(text, 64, "%7.3f", (float)entry->offset);		
			psy_ui_label_settext(&self->offset.second, text);
		}
	}
}

void seqedittimesigproperties_onnominator(SeqEditTimesigProperties* self, IntEdit* sender)
{
	psy_audio_Pattern* pattern;

	pattern = psy_audio_patterns_at(
		&self->state->cmds->workspace->song->patterns,
		psy_audio_GLOBALPATTERN);				
	if (pattern) {
		psy_audio_PatternNode* node;
		psy_audio_PatternEntry* entry;		
						
		node = psy_audio_pattern_timesig_at(pattern, self->timesigindex,
			psy_audio_GLOBALPATTERN_TIMESIGTRACK);
		if (node) {
			psy_audio_PatternEvent* e;

			entry = (psy_audio_PatternEntry*)(node->entry); 
			e = psy_audio_patternentry_front(entry);
			e->cmd = intedit_value(sender);
			psy_signal_emit(&self->state->signal_timesigchanged, self->state, 0);
		}		
	}
}

void seqedittimesigproperties_ondenominator(SeqEditTimesigProperties* self, IntEdit* sender)
{
	psy_audio_Pattern* pattern;

	pattern = psy_audio_patterns_at(
		&self->state->cmds->workspace->song->patterns,
		psy_audio_GLOBALPATTERN);				
	if (pattern) {
		psy_audio_PatternNode* node;
		psy_audio_PatternEntry* entry;		
						
		node = psy_audio_pattern_timesig_at(pattern, self->timesigindex,
			psy_audio_GLOBALPATTERN_TIMESIGTRACK);
		if (node) {
			psy_audio_PatternEvent* e;

			entry = (psy_audio_PatternEntry*)(node->entry); 
			e = psy_audio_patternentry_front(entry);
			e->parameter = intedit_value(sender);
			psy_signal_emit(&self->state->signal_timesigchanged, self->state, 0);
		}		
	}
}


/* SeqEditLoopProperties */
/* prototypes */
static void seqeditloopproperties_onnumloops(SeqEditLoopProperties*, IntEdit* sender);

/*  implementation */
void seqeditloopproperties_init(SeqEditLoopProperties* self, psy_ui_Component* parent,
	SeqEditState* state)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	self->state = state;		
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(40.0, 20.0));
	psy_ui_component_setspacing(&self->component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));
	psy_ui_label_init(&self->caption, &self->component);
	psy_ui_label_settext(&self->caption, "Loop Item selected");
	psy_ui_component_setalign(psy_ui_label_base(&self->caption),
		psy_ui_ALIGN_TOP);	
	intedit_init(&self->numloops, &self->component, "Repetitions", 1, 1, 0xF);
	psy_ui_component_setalign(intedit_base(&self->numloops),
		psy_ui_ALIGN_TOP);
	psy_ui_label_setcharnumber(&self->numloops.desc, 12.0);
	psy_ui_component_setmargin(&self->numloops.component,
		psy_ui_margin_make_em(0.5, 0.0, 0.0, 0.0));	
	/* position */
	labelpair_init(&self->offset, &self->component, "Position", 12.0);
	psy_ui_component_setalign(psy_ui_label_base(&self->offset.second),
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(labelpair_base(&self->offset), psy_ui_ALIGN_TOP);
	/* length */
	labelpair_init(&self->length, &self->component, "Length", 12.0);
	psy_ui_component_setalign(psy_ui_label_base(&self->length.second),
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(labelpair_base(&self->length), psy_ui_ALIGN_TOP);
	/* end */
	labelpair_init(&self->end, &self->component, "End", 12.0);
	psy_ui_component_setalign(psy_ui_label_base(&self->end.second),
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(labelpair_base(&self->end), psy_ui_ALIGN_TOP);	

	seqeditloopproperties_setloopindex(self, psy_INDEX_INVALID);	
	psy_signal_connect(&self->numloops.signal_changed, self,
		seqeditloopproperties_onnumloops);	
}

void seqeditloopproperties_onnumloops(SeqEditLoopProperties* self, IntEdit* sender)
{
	psy_audio_Pattern* pattern;

	pattern = psy_audio_patterns_at(
		&self->state->cmds->workspace->song->patterns,
		psy_audio_GLOBALPATTERN);				
	if (pattern) {
		psy_audio_Loop loop;
		psy_audio_PatternEntry* entry;		
						
		loop = psy_audio_pattern_loop_at(pattern, self->loopindex,
			psy_audio_GLOBALPATTERN_LOOPTRACK);
		if (loop.end) {
			psy_audio_PatternEvent* e;

			entry = (psy_audio_PatternEntry*)(loop.end->entry); 
			e = psy_audio_patternentry_front(entry);
			e->parameter = 0xB0 | (uint8_t)intedit_value(sender);
			psy_signal_emit(&self->state->signal_loopchanged, self->state, 0);
		}		
	}
}

void seqeditloopproperties_setloopindex(SeqEditLoopProperties* self, uintptr_t loopindex)
{
	psy_audio_Pattern* pattern;

	self->loopindex = loopindex;	
	pattern = psy_audio_patterns_at(
		&self->state->cmds->workspace->song->patterns,
		psy_audio_GLOBALPATTERN);			
	if (loopindex == psy_INDEX_INVALID) {						
		psy_ui_label_settext(&self->caption, "No Loop selected");	
		return;
	} else {
		psy_ui_label_settext(&self->caption, "Loop Item selected");	
	}
	if (pattern) {
		char text[64];
		psy_audio_Loop loop;
		psy_audio_PatternEntry* start;
		psy_audio_PatternEntry* end;
		psy_audio_PatternEvent e;
					
		loop = psy_audio_pattern_loop_at(pattern, loopindex,
			psy_audio_GLOBALPATTERN_LOOPTRACK);
		if (loop.start) {
			start = (psy_audio_PatternEntry*)(loop.start->entry);
		} else {
			start = NULL;
		}
		if (loop.end) {
			end = (psy_audio_PatternEntry*)(loop.end->entry);
		} else {
			end = NULL;
		}
		if (end) {		
			e = *psy_audio_patternentry_front(end);
			intedit_setvalue(&self->numloops, (e.parameter & 0x0F));
		}
		if (start) {			
			psy_snprintf(text, 64, "%7.3f", (float)start->offset);
			psy_ui_label_settext(&self->offset.second, text);
		} else {
			psy_ui_label_settext(&self->offset.second, "");
		}
		if (start && end) {		
			psy_snprintf(text, 64, "%7.3f", (float)end->offset - start->offset);
			psy_ui_label_settext(&self->length.second, text);
		} else {
			psy_ui_label_settext(&self->length.second, "");
		}
		if (end) {
			psy_snprintf(text, 64, "%7.3f", (float)end->offset);
			psy_ui_label_settext(&self->end.second, text);
		} else {
			psy_ui_label_settext(&self->end.second, "");
		}
	}
}

/* SeqEditProperties */
/* prototypes */
static void seqeditproperties_ondestroy(SeqEditProperties*);
static void seqeditproperties_onitemselected(SeqEditProperties*,
	SeqEditState* sender, uintptr_t itemtype, uintptr_t param, uintptr_t param1);
static void seqeditproperties_onsequencetrackreposition(SeqEditProperties*,
	psy_audio_Sequence* sender, uintptr_t trackidx);
static void seqeditproperties_onsongchanged(SeqEditProperties*,
	Workspace* sender);
static void seqeditproperties_connectsong(SeqEditProperties*);

/* vtable */
static psy_ui_ComponentVtable seqeditproperties_vtable;
static bool seqeditproperties_vtable_initialized = FALSE;

static void seqeditproperties_vtable_init(SeqEditProperties* self)
{
	if (!seqeditproperties_vtable_initialized) {
		seqeditproperties_vtable = *(self->component.vtable);		
		seqeditproperties_vtable.ondestroy =
			(psy_ui_fp_component_event)
			seqeditproperties_ondestroy;
		seqeditproperties_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqeditproperties_vtable;
}
/*  implementation */
void seqeditproperties_init(SeqEditProperties* self, psy_ui_Component* parent,
	SeqEditState* state)
{	
	psy_ui_component_init(&self->component, parent, NULL);	
	seqeditproperties_vtable_init(self);
	self->state = state;
	self->itemtype = SEQEDITITEM_NONE;
	self->param1 = 0;
	self->param2 = 0;		
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(30.0, 20.0));
	psy_ui_component_setspacing(&self->component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));	
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	seqeditemptyproperties_init(&self->empty, psy_ui_notebook_base(&self->notebook),
		self->state);
	seqeditpatternproperties_init(&self->pattern, psy_ui_notebook_base(&self->notebook),
		self->state);	
	seqeditsampleproperties_init(&self->sample, psy_ui_notebook_base(&self->notebook),
		self->state);	
	seqeditmarkerproperties_init(&self->marker, psy_ui_notebook_base(&self->notebook),
		self->state);
	seqedittimesigproperties_init(&self->timesig, psy_ui_notebook_base(&self->notebook),
		self->state);
	seqeditloopproperties_init(&self->loop, psy_ui_notebook_base(&self->notebook),
		self->state);
	seqeditproperties_selectitem(self, SEQEDITITEM_NONE, 0, 0);
	psy_signal_connect(&self->state->signal_itemselected, self,
		seqeditproperties_onitemselected);
	psy_signal_connect(&self->state->cmds->workspace->signal_songchanged, self,
		seqeditproperties_onsongchanged);
	seqeditproperties_connectsong(self);
}

void seqeditproperties_ondestroy(SeqEditProperties* self)
{

}

void seqeditproperties_selectitem(SeqEditProperties* self, SeqEditItemType type,
	uintptr_t param1, uintptr_t param2)
{
	uintptr_t pageindex;

	self->itemtype = type;
	pageindex = 0;
	self->param1 = param1;
	self->param2 = param2;
	switch (self->itemtype) {	
	case SEQEDITITEM_PATTERN:
		seqeditpatternproperties_setorderindex(&self->pattern,
			psy_audio_orderindex_make(param1, param2));
		pageindex = 1;
		break;
	case SEQEDITITEM_SAMPLE:
		seqeditsampleproperties_setorderindex(&self->sample,
			psy_audio_orderindex_make(param1, param2));
		pageindex = 2;
		break;
	case SEQEDITITEM_MARKER:
		seqeditmarkerproperties_setorderindex(&self->marker,
			psy_audio_orderindex_make(param1, param2));
		pageindex = 3;
		break;
	case SEQEDITITEM_TIMESIG: {		
		seqedittimesigproperties_settimesigindex(&self->timesig, param1);
		pageindex = 4;
		break; }
	case SEQEDITITEM_LOOP: {		
		seqeditloopproperties_setloopindex(&self->loop, param1);
		pageindex = 5;
		break; }
	case SEQEDITITEM_NONE:
	default:		
		pageindex = 0;
		psy_ui_notebook_select(&self->notebook, 0);
		break;
	}
	if (psy_ui_notebook_pageindex(&self->notebook) != pageindex) {
		psy_ui_notebook_select(&self->notebook, pageindex);
	}
}

void seqeditproperties_onitemselected(SeqEditProperties* self,
	SeqEditState* sender, uintptr_t itemtype, uintptr_t param1, uintptr_t param2)
{
	seqeditproperties_selectitem(self, (SeqEditItemType)itemtype, param1, param2);
}

void seqeditproperties_onsongchanged(SeqEditProperties* self, Workspace* sender)
{
	seqeditproperties_connectsong(self);
	seqeditproperties_selectitem(self, SEQEDITITEM_NONE, 0, 0);
}

void seqeditproperties_connectsong(SeqEditProperties* self)
{
	psy_audio_Song* song;

	song = workspace_song(self->state->cmds->workspace);
	if (song) {		
		psy_signal_connect(&song->sequence.signal_trackreposition,
			self, seqeditproperties_onsequencetrackreposition);
	}
}

void seqeditproperties_onsequencetrackreposition(SeqEditProperties* self,
	psy_audio_Sequence* sender, uintptr_t trackidx)
{
	seqeditproperties_selectitem(self, self->itemtype, self->param1,
		self->param2);
}
