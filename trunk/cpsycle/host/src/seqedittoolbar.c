/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqedittoolbar.h"
/* host */
#include "resources/resource.h"
#include "styles.h"
/* audio */
#include <exclusivelock.h>
/* platform */
#include "../../detail/portable.h"

/* SeqEditToolBar */
static void seqedittoolbar_oninserttypeselchange(SeqEditToolBar*,
	psy_ui_Component* sender, intptr_t index);
static void seqeditortoolbar_ondragmodemove(SeqEditToolBar*,
	psy_ui_Component* sender);
static void seqeditortoolbar_ondragmodereorder(SeqEditToolBar*,
	psy_ui_Component* sender);
static void seqeditortoolbar_onsequenceselectionselect(SeqEditToolBar*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditortoolbar_onsequenceselectiondeselect(SeqEditToolBar*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditortoolbar_onassignsample(SeqEditToolBar*,
	psy_ui_Component* sender);
static void seqeditortoolbar_onusesamplerindex(SeqEditToolBar*,
	psy_ui_Component* sender);
static void seqeditortoolbar_onsamplerindexchange(SeqEditToolBar*,
	IntEdit* sender);
static void seqeditortoolbar_onconfigure(SeqEditToolBar*,
	psy_ui_Button* sender);
static psy_audio_SequenceSampleEntry* seqeditortoolbar_sampleentry(
	SeqEditToolBar* self);
static void seqeditortoolbar_ontrackeditaccept(SeqEditToolBar*,
	psy_ui_TextArea* sender);
static void seqeditortoolbar_ontrackeditreject(SeqEditToolBar*,
	psy_ui_TextArea* sender);

/* implenentation */
void seqedittoolbar_init(SeqEditToolBar* self, psy_ui_Component* parent,
	SeqEditState* state)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_style_type(&self->component, STYLE_SEQEDT_TOOLBAR);		
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make_em(0.0, 0.5, 0.0, 1.0));		
	self->state = state;
	psy_ui_label_init_text(&self->trackname, &self->component, "Track");
	psy_ui_textarea_init_single_line(&self->trackedit, &self->component);	
	psy_ui_textarea_set_char_number(&self->trackedit, 30);
	psy_ui_textarea_enable_input_field(&self->trackedit);	
	psy_signal_connect(&self->trackedit.signal_accept,
		self, seqeditortoolbar_ontrackeditaccept);
	psy_signal_connect(&self->trackedit.signal_reject,
		self, seqeditortoolbar_ontrackeditreject);
	psy_ui_button_init_text(&self->move, &self->component,
		"seqedit.move");
	psy_ui_button_init_text(&self->reorder, &self->component,
		"seqedit.reorder");
	seqeditortoolbar_setdragtype(self, self->state->dragtype);
	psy_ui_label_init_text(&self->desctype, seqedittoolbar_base(self),
		"Insert");	
	psy_ui_combobox_init(&self->inserttype, seqedittoolbar_base(self));
	psy_ui_combobox_setcharnumber(&self->inserttype, 12.0);
	psy_ui_combobox_add_text(&self->inserttype, "Pattern");
	psy_ui_combobox_add_text(&self->inserttype, "Marker");
	psy_ui_combobox_add_text(&self->inserttype, "Sample");
	psy_ui_combobox_setcursel(&self->inserttype, 0);
	/* expand */
	psy_ui_button_init(&self->expand, &self->component);
	psy_ui_button_load_resource(&self->expand, IDB_EXPAND_LIGHT,
		IDB_EXPAND_DARK, psy_ui_colour_white());
	psy_ui_component_set_align(psy_ui_button_base(&self->expand),
		psy_ui_ALIGN_RIGHT);	
	/* configure */
	psy_ui_button_init_connect(&self->configure, &self->component,
		self, seqeditortoolbar_onconfigure);
	psy_ui_button_load_resource(&self->configure, IDB_SETTINGS_LIGHT,
		IDB_SETTINGS_DARK, psy_ui_colour_white());
	psy_ui_component_set_align(psy_ui_button_base(&self->configure),
		psy_ui_ALIGN_RIGHT);
	/* assign sample */
	psy_ui_button_init_text_connect(&self->assignsample, &self->component,
		"Assign Sample", self, seqeditortoolbar_onassignsample);
	psy_ui_component_hide(psy_ui_button_base(&self->assignsample));
	psy_ui_checkbox_init_text(&self->usesamplerindex, &self->component, "Define Sampler");
	psy_signal_connect(&self->usesamplerindex.signal_clicked, self,
		seqeditortoolbar_onusesamplerindex);
	intedit_init_connect(&self->samplerindex, &self->component, "Index",
		0x3E, 0, 0x3F, self, seqeditortoolbar_onsamplerindexchange);	
	psy_ui_component_hide(psy_ui_checkbox_base(&self->usesamplerindex));
	psy_ui_component_hide(intedit_base(&self->samplerindex));	
	/* timesig */
	psy_ui_button_init_text(&self->timesig, &self->component, "Timesigs");
	psy_ui_button_seticon(&self->timesig, psy_ui_ICON_MORE);
	/* loop */
	psy_ui_button_init_text(&self->loop, &self->component, "Repetitions");
	psy_ui_button_seticon(&self->timesig, psy_ui_ICON_MORE);
	psy_signal_connect(&self->inserttype.signal_selchanged, self,
		seqedittoolbar_oninserttypeselchange);
	psy_signal_connect(&self->move.signal_clicked, self,
		seqeditortoolbar_ondragmodemove);
	psy_signal_connect(&self->reorder.signal_clicked, self,
		seqeditortoolbar_ondragmodereorder);
	psy_signal_connect(&state->workspace->song->sequence.sequenceselection.signal_select,
		self, seqeditortoolbar_onsequenceselectionselect);
	psy_signal_connect(&state->workspace->song->sequence.sequenceselection.signal_deselect,
		self, seqeditortoolbar_onsequenceselectiondeselect);
}

void seqeditortoolbar_ondragmodemove(SeqEditToolBar* self,
	psy_ui_Component* sender)
{	
	seqeditortoolbar_setdragtype(self, SEQEDIT_DRAGTYPE_MOVE);
}

void seqeditortoolbar_ondragmodereorder(SeqEditToolBar* self,
	psy_ui_Component* sender)
{	
	seqeditortoolbar_setdragtype(self, SEQEDIT_DRAGTYPE_REORDER);
}

void seqeditortoolbar_setdragtype(SeqEditToolBar* self, SeqEditorDragType type)
{
	self->state->dragtype = type;
	switch (type) {
	case SEQEDIT_DRAGTYPE_MOVE:
		psy_ui_button_highlight(&self->move);
		psy_ui_button_disable_highlight(&self->reorder);
		break;
	case SEQEDIT_DRAGTYPE_REORDER:
		psy_ui_button_highlight(&self->reorder);
		psy_ui_button_disable_highlight(&self->move);
		break;
	default:
		break;
	}
}

void seqedittoolbar_oninserttypeselchange(SeqEditToolBar* self,
	psy_ui_Component* sender, intptr_t index)
{
	switch (index) {
	case 0:
		self->state->inserttype = psy_audio_SEQUENCEENTRY_PATTERN;
		break;
	case 1:
		self->state->inserttype = psy_audio_SEQUENCEENTRY_MARKER;
		break;
	case 2:
		self->state->inserttype = psy_audio_SEQUENCEENTRY_SAMPLE;
		break;
	default:
		self->state->inserttype = psy_audio_SEQUENCEENTRY_PATTERN;
		break;
	}
}

void seqeditortoolbar_onsequenceselectionselect(SeqEditToolBar* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{
	psy_audio_SequenceEntry* entry;
	psy_audio_SequenceSampleEntry* sampleentry;

	entry = psy_audio_sequence_entry(seqeditstate_sequence(self->state),
		*index);
	if (entry && entry->type == psy_audio_SEQUENCEENTRY_SAMPLE) {
		if (!psy_ui_component_visible(psy_ui_button_base(&self->assignsample))) {
			psy_ui_component_show(psy_ui_checkbox_base(&self->usesamplerindex));
			psy_ui_component_show(intedit_base(&self->samplerindex));			
			psy_ui_component_show_align(psy_ui_button_base(&self->assignsample));			
		}
	} else {
		if (psy_ui_component_visible(psy_ui_button_base(&self->assignsample))) {
			psy_ui_component_hide(psy_ui_checkbox_base(&self->usesamplerindex));
			psy_ui_component_hide(intedit_base(&self->samplerindex));			
			psy_ui_component_hide_align(psy_ui_button_base(&self->assignsample));
		}		
	}	
	sampleentry = seqeditortoolbar_sampleentry(self);
	if (sampleentry) {
		if (sampleentry->samplerindex != psy_INDEX_INVALID) {
			psy_ui_checkbox_check(&self->usesamplerindex);
			intedit_set_value(&self->samplerindex,
				(int)sampleentry->samplerindex);			
			psy_ui_component_show_align(intedit_base(&self->samplerindex));
		} else {			
			psy_ui_component_hide_align(intedit_base(&self->samplerindex));
		}
	}
	seqeditortoolbar_updatetrackname(self);
}

void seqeditortoolbar_onsequenceselectiondeselect(SeqEditToolBar* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{
	seqeditortoolbar_updatetrackname(self);
}

void seqeditortoolbar_updatetrackname(SeqEditToolBar* self)
{
	psy_audio_OrderIndex editposition;
	psy_audio_SequenceTrack* track;
	psy_audio_Sequence* sequence;

	sequence = seqeditstate_sequence(self->state);
	editposition = seqeditstate_editposition(self->state);
	track = psy_audio_sequence_track_at(sequence, editposition.track);
	if (track) {
		psy_ui_textarea_set_text(&self->trackedit, track->name);
	} else {
		psy_ui_textarea_set_text(&self->trackedit, "");
	}
}


void seqeditortoolbar_onassignsample(SeqEditToolBar* self,
	psy_ui_Component* sender)
{
	psy_audio_SequenceSampleEntry* sampleentry;

	sampleentry = seqeditortoolbar_sampleentry(self);
	if (sampleentry) {
		psy_audio_OrderIndex editposition;
		psy_audio_SampleIndex sampleindex;
		psy_audio_SequenceTrack* track;
		psy_audio_Sequence* sequence;

		sequence = seqeditstate_sequence(self->state);
		sampleindex = self->state->cmds->workspace->song->samples.selected;
		editposition = seqeditstate_editposition(self->state);
		psy_audio_sequencesampleentry_setsampleslot(sampleentry, sampleindex);
		track = psy_audio_sequence_track_at(sequence, editposition.track);
		if (track) {
			psy_audio_sequence_reposition_track(sequence, track);
		}
	}	
}

void seqeditortoolbar_onusesamplerindex(SeqEditToolBar* self,
	psy_ui_Component* sender)
{
	psy_audio_SequenceSampleEntry* sampleentry;

	sampleentry = seqeditortoolbar_sampleentry(self);
	if (sampleentry) {
		if (!psy_ui_checkbox_checked(&self->usesamplerindex)) {			
			psy_audio_sequencesampleentry_setsamplerindex(sampleentry,
				psy_INDEX_INVALID);
		} else {
			psy_audio_sequencesampleentry_setsamplerindex(sampleentry,
				intedit_value(&self->samplerindex));			
		}
		if (sampleentry->samplerindex != psy_INDEX_INVALID) {						
			psy_ui_component_show_align(intedit_base(&self->samplerindex));			
		} else {			
			psy_ui_component_hide_align(intedit_base(&self->samplerindex));
		}
	}
}

void seqeditortoolbar_onsamplerindexchange(SeqEditToolBar* self,
	IntEdit* sender)
{
	psy_audio_SequenceSampleEntry* sampleentry;

	sampleentry = seqeditortoolbar_sampleentry(self);
	if (sampleentry && (psy_ui_checkbox_checked(&self->usesamplerindex))) {
		psy_audio_sequencesampleentry_setsamplerindex(sampleentry,
			intedit_value(&self->samplerindex));
	}
}

void seqeditortoolbar_onconfigure(SeqEditToolBar* self, psy_ui_Button* sender)
{
	workspace_select_view(self->state->workspace,
		viewindex_make(VIEW_ID_SETTINGSVIEW, 11, 0, psy_INDEX_INVALID));
}

psy_audio_SequenceSampleEntry* seqeditortoolbar_sampleentry(SeqEditToolBar* self)
{
	psy_audio_Sequence* sequence;

	sequence = seqeditstate_sequence(self->state);
	if (sequence) {		
		psy_audio_SequenceEntry* seqentry;
				
		seqentry = psy_audio_sequence_entry(sequence,
			seqeditstate_editposition(self->state));
		if (seqentry && seqentry->type == psy_audio_SEQUENCEENTRY_SAMPLE) {
			return (psy_audio_SequenceSampleEntry*)seqentry;			 
		}
	}
	return NULL;
}

void seqeditortoolbar_ontrackeditaccept(SeqEditToolBar* self,
	psy_ui_TextArea* sender)
{
	psy_audio_OrderIndex editposition;
	psy_audio_SequenceTrack* track;
	psy_audio_Sequence* sequence;

	sequence = seqeditstate_sequence(self->state);
	editposition = seqeditstate_editposition(self->state);
	track = psy_audio_sequence_track_at(sequence, editposition.track);
	if (track) {
		psy_audio_sequencetrack_setname(track, psy_ui_textarea_text(sender));
		psy_signal_emit(&sequence->signal_clear, sequence, 0);
	}
	psy_ui_component_set_focus(self->state->view);
}

void seqeditortoolbar_ontrackeditreject(SeqEditToolBar* self,
	psy_ui_TextArea* sender)
{
	psy_ui_component_set_focus(self->state->view);
}
