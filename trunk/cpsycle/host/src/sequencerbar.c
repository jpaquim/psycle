/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sequencerbar.h"
/* host */
#include "styles.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

/* prototypes */
static void sequencerbar_on_show_pattern_names(SequencerBar*,
	psy_ui_CheckBox* sender);
static void sequencerbar_on_follow_song(SequencerBar*, psy_ui_Button* sender);
static void sequencerbar_on_record_tweak(SequencerBar*, psy_ui_Button* sender);
static void sequencerbar_on_record_noteoff(SequencerBar*, psy_ui_Button* sender);
static void sequencerbar_on_multi_channel_audition(SequencerBar*,
	psy_ui_Button* sender);
static void sequencerbar_on_configure(SequencerBar*, GeneralConfig*,
	psy_Property*);
void sequencerbar_on_misc_configure(SequencerBar*, KeyboardMiscConfig*,
	psy_Property*);

/* implementation */
void sequencerbar_init(SequencerBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;

	assert(self);
	assert(workspace);
	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_style_type(&self->component, STYLE_SEQBAR);
	psy_ui_margin_init_em(&margin, 0.0, 0.0, 0.25, 0.0);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		margin);
	self->workspace = workspace;
	psy_ui_checkbox_init_text(&self->follow_song, &self->component,	
		"seqview.follow-song");	
	psy_ui_checkbox_init_text(&self->shownames, &self->component,	
		"seqview.show-pattern-names");			
	psy_ui_checkbox_init_text(&self->recordnoteoff, &self->component,
		"seqview.record-noteoff");	
	psy_ui_checkbox_init_text(&self->recordtweak, &self->component,
		"seqview.record-tweak");	
	psy_ui_checkbox_init_text(&self->multichannelaudition, &self->component,
		"seqview.multichannel-audition");			
	psy_ui_checkbox_init_text(&self->allownotestoeffect, &self->component,
		"seqview.allow-notes-to_effect");						
	psy_ui_button_init_text(&self->togglestepseq, &self->component,
		"seqview.showstepsequencer");
	psy_ui_button_seticon(&self->togglestepseq, psy_ui_ICON_MORE);
	psy_ui_button_settextalignment(&self->togglestepseq,
		(psy_ui_Alignment)(psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_CENTER_VERTICAL));
	psy_ui_component_set_align(&self->togglestepseq.component,
		psy_ui_ALIGN_BOTTOM);	
	psy_ui_button_init_text(&self->toggleseqedit, &self->component,
		"seqview.showseqeditor");
	psy_ui_component_set_align(&self->toggleseqedit.component,
		psy_ui_ALIGN_BOTTOM);
	psy_ui_button_settextalignment(&self->toggleseqedit,
		(psy_ui_Alignment)(psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_CENTER_VERTICAL));
	psy_ui_button_seticon(&self->toggleseqedit, psy_ui_ICON_MORE);
	psy_ui_margin_init_em(&margin, 0.0, 0.0, 1.0, 0.0);
	psy_ui_component_set_margin(&self->allownotestoeffect.component, margin);
	psy_signal_connect(&self->follow_song.signal_clicked, self,
		sequencerbar_on_follow_song);
	psy_signal_connect(&self->shownames.signal_clicked, self,
		sequencerbar_on_show_pattern_names);
	psy_signal_connect(&self->recordnoteoff.signal_clicked, self,
		sequencerbar_on_record_noteoff);
	psy_signal_connect(&self->recordtweak.signal_clicked, self,
		sequencerbar_on_record_tweak);
	psy_signal_connect(&self->multichannelaudition.signal_clicked, self,
		sequencerbar_on_multi_channel_audition);
	psy_signal_connect(
		&psycleconfig_general(workspace_conf(workspace))->signal_changed,
		self, sequencerbar_on_configure);
	psy_signal_connect(
		&psycleconfig_misc(workspace_conf(workspace))->signal_changed,
		self, sequencerbar_on_misc_configure);
}

void sequencerbar_on_follow_song(SequencerBar* self, psy_ui_Button* sender)
{
	if (keyboardmiscconfig_following_song(&self->workspace->config.misc)) {
		keyboardmiscconfig_stop_follow_song(&self->workspace->config.misc);		
	} else {
		keyboardmiscconfig_follow_song(&self->workspace->config.misc);		
	}
}

void sequencerbar_on_show_pattern_names(SequencerBar* self, psy_ui_CheckBox* sender)
{
	if (psy_ui_checkbox_checked(sender) != 0) {
		generalconfig_show_pattern_names(psycleconfig_general(
			workspace_conf(self->workspace)));		
	} else {
		generalconfig_show_pattern_ids(psycleconfig_general(
			workspace_conf(self->workspace)));		
	}
}

void sequencerbar_on_record_tweak(SequencerBar* self, psy_ui_Button* sender)
{
	if (workspace_recording_tweaks(self->workspace)) {
		workspace_stop_record_tweaks(self->workspace);
	} else {
		workspace_record_tweaks(self->workspace);
	}
}

void sequencerbar_on_record_noteoff(SequencerBar* self, psy_ui_Button* sender)
{
	if (psy_audio_player_recording_noteoff(workspace_player(self->workspace))) {
		psy_audio_player_preventrecordnoteoff(workspace_player(self->workspace));
	} else {
		psy_audio_player_recordnoteoff(workspace_player(self->workspace));
	}
}

void sequencerbar_on_multi_channel_audition(SequencerBar* self, psy_ui_Button* sender)
{
	workspace_player(self->workspace)->multichannelaudition =
		!workspace_player(self->workspace)->multichannelaudition;
}

void sequencerbar_on_configure(SequencerBar* self, GeneralConfig* config,
	psy_Property* property)
{	
	if (generalconfig_showingpatternnames(config)) {
		psy_ui_checkbox_check(&self->shownames);
	} else {
		psy_ui_checkbox_disablecheck(&self->shownames);
	}	
}

void sequencerbar_on_misc_configure(SequencerBar* self, KeyboardMiscConfig* config,
	psy_Property* property)
{
	if (keyboardmiscconfig_following_song(config)) {
		psy_ui_checkbox_check(&self->follow_song);
	} else {
		psy_ui_checkbox_disablecheck(&self->follow_song);
	}
}