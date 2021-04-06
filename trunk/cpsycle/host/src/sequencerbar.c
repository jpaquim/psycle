// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequencerbar.h"
// host
#include "styles.h"
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// prototypes
static void sequencerbar_onshowpatternnames(SequencerBar*,
	psy_ui_CheckBox* sender);
static void sequencerbar_onfollowsong(SequencerBar*, psy_ui_Button* sender);
static void sequencerbar_onfollowsongchanged(SequencerBar*, Workspace* sender);
static void sequencerbar_onrecordtweak(SequencerBar*, psy_ui_Button* sender);
static void sequencerbar_onrecordnoteoff(SequencerBar*, psy_ui_Button* sender);
static void sequencerbar_onmultichannelaudition(SequencerBar*,
	psy_ui_Button* sender);
static void sequencerbar_onconfigure(SequencerBar*, GeneralConfig*,
	psy_Property*);
// implementation
void sequencerbar_init(SequencerBar* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_Margin margin;

	assert(self);
	assert(workspace);
	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 0.25, 0.0);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_TOP,
		margin);
	self->workspace = workspace;
	psy_ui_checkbox_init_multiline(&self->followsong, &self->component);	
	psy_ui_checkbox_settext(&self->followsong,
		"sequencerview.follow-song");
	psy_ui_checkbox_init_multiline(&self->shownames, &self->component);	
	psy_ui_checkbox_settext(&self->shownames,
		"sequencerview.show-pattern-names");			
	psy_ui_checkbox_init_multiline(&self->recordnoteoff, &self->component);
	psy_ui_checkbox_settext(&self->recordnoteoff,
		"sequencerview.record-noteoff");	
	psy_ui_checkbox_init_multiline(&self->recordtweak, &self->component);
	psy_ui_checkbox_settext(&self->recordtweak,
		"sequencerview.record-tweak");	
	psy_ui_checkbox_init_multiline(&self->multichannelaudition, &self->component);
	psy_ui_checkbox_settext(&self->multichannelaudition,
		"sequencerview.multichannel-audition");			
	psy_ui_checkbox_init_multiline(&self->allownotestoeffect, &self->component);
	psy_ui_checkbox_settext(&self->allownotestoeffect,
		"sequencerview.allow-notes-to_effect");	
				
	psy_ui_component_init(&self->stepseq, &self->component, NULL);
	psy_ui_component_setalign(&self->stepseq, psy_ui_ALIGN_BOTTOM);		
	psy_ui_button_init_text(&self->togglestepseq, &self->stepseq, NULL,
		"sequencerview.showstepsequencer");
	psy_ui_button_seticon(&self->togglestepseq, psy_ui_ICON_MORE);
	psy_ui_component_setalign(&self->togglestepseq.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_component_init(&self->seqedit, &self->component, NULL);
	psy_ui_component_setalign(&self->seqedit, psy_ui_ALIGN_BOTTOM);
	psy_ui_button_init_text(&self->toggleseqedit, &self->seqedit, NULL,
		"sequencerview.showseqeditor");
	psy_ui_component_setalign(&self->toggleseqedit.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_button_seticon(&self->toggleseqedit, psy_ui_ICON_MORE);

	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 1.0, 0.0);
	psy_ui_component_setmargin(&self->allownotestoeffect.component, &margin);
	psy_signal_connect(&self->followsong.signal_clicked, self,
		sequencerbar_onfollowsong);
	psy_signal_connect(&workspace->signal_followsongchanged, self,
		sequencerbar_onfollowsongchanged);	
	psy_signal_connect(&self->shownames.signal_clicked, self,
		sequencerbar_onshowpatternnames);
	psy_signal_connect(&self->recordnoteoff.signal_clicked, self,
		sequencerbar_onrecordnoteoff);
	psy_signal_connect(&self->recordtweak.signal_clicked, self,
		sequencerbar_onrecordtweak);
	psy_signal_connect(&self->multichannelaudition.signal_clicked, self,
		sequencerbar_onmultichannelaudition);
	psy_signal_connect(
		&psycleconfig_general(workspace_conf(workspace))->signal_changed,
		self, sequencerbar_onconfigure);
}

void sequencerbar_onfollowsong(SequencerBar* self, psy_ui_Button* sender)
{
	if (workspace_followingsong(self->workspace)) {
		workspace_stopfollowsong(self->workspace);
	} else {
		workspace_followsong(self->workspace);
	}
}

void sequencerbar_onfollowsongchanged(SequencerBar* self, Workspace* sender)
{
	if (workspace_followingsong(sender)) {		
		psy_ui_checkbox_check(&self->followsong);
	} else {
		psy_ui_checkbox_disablecheck(&self->followsong);
	}
}

void sequencerbar_onshowpatternnames(SequencerBar* self, psy_ui_CheckBox* sender)
{
	if (psy_ui_checkbox_checked(sender) != 0) {
		generalconfig_showpatternnames(psycleconfig_general(
			workspace_conf(self->workspace)));		
	} else {
		generalconfig_showpatternids(psycleconfig_general(
			workspace_conf(self->workspace)));		
	}
}

void sequencerbar_onrecordtweak(SequencerBar* self, psy_ui_Button* sender)
{
	if (workspace_recordingtweaks(self->workspace)) {
		workspace_stoprecordtweaks(self->workspace);
	} else {
		workspace_recordtweaks(self->workspace);
	}
}

void sequencerbar_onrecordnoteoff(SequencerBar* self, psy_ui_Button* sender)
{
	if (psy_audio_player_recordingnoteoff(workspace_player(self->workspace))) {
		psy_audio_player_preventrecordnoteoff(workspace_player(self->workspace));
	} else {
		psy_audio_player_recordnoteoff(workspace_player(self->workspace));
	}
}

void sequencerbar_onmultichannelaudition(SequencerBar* self, psy_ui_Button* sender)
{
	workspace_player(self->workspace)->multichannelaudition =
		!workspace_player(self->workspace)->multichannelaudition;
}

void sequencerbar_onconfigure(SequencerBar* self, GeneralConfig* config,
	psy_Property* property)
{	
	if (generalconfig_showingpatternnames(config)) {
		psy_ui_checkbox_check(&self->shownames);
	} else {
		psy_ui_checkbox_disablecheck(&self->shownames);
	}	
}
