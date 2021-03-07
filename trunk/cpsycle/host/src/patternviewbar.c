// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternviewbar.h"
// audio
#include <patternio.h>
#include <songio.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"

// PatternViewBar
// prototypes
static void patternviewbar_onconfigure(PatternViewBar*, PatternViewConfig*,
	psy_Property*);
static void patternviewbar_onmovecursorwhenpaste(PatternViewBar*,
	psy_ui_Component* sender);
static void patternviewbar_ondefaultline(PatternViewBar*,
	psy_ui_CheckBox* sender);
static void patternviewbar_ondisplaysinglepattern(PatternViewBar*,
	psy_ui_CheckBox* sender);
static void patternviewbar_onupdatestatus(PatternViewBar*,
	Workspace* sender);
static void patternviewbar_onsequenceselectionchanged(PatternViewBar*,
	psy_audio_SequenceSelection* sender);
static void patternviewbar_onsongchanged(PatternViewBar*, Workspace* sender,
	int flag, psy_audio_Song*);
static void patternviewbar_updatestatus(PatternViewBar*);
// implementation
void patternviewbar_init(PatternViewBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{		
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	psy_ui_component_setdefaultalign(patternviewbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	patterncursorstepbox_init(&self->cursorstep, &self->component, workspace);
	// Move cursor when paste
	psy_ui_checkbox_init(&self->movecursorwhenpaste, patternviewbar_base(self));
	psy_ui_checkbox_settext(&self->movecursorwhenpaste,
		"settingsview.move-cursor-when-paste");	
	psy_signal_connect(&self->movecursorwhenpaste.signal_clicked, self,
		patternviewbar_onmovecursorwhenpaste);
	// Default line
	psy_ui_checkbox_init(&self->defaultentries, patternviewbar_base(self));
	psy_ui_checkbox_settext(&self->defaultentries,
		"settingsview.default-line");
	if (patternviewconfig_defaultline(psycleconfig_patview(
			workspace_conf(workspace)))) {
		psy_ui_checkbox_check(&self->defaultentries);
	}
	psy_signal_connect(&self->defaultentries.signal_clicked, self,
		patternviewbar_ondefaultline);
	// Single pattern display mode
	psy_ui_checkbox_init(&self->displaysinglepattern, patternviewbar_base(self));
	psy_ui_checkbox_settext(&self->displaysinglepattern,
		"settingsview.displaysinglepattern");
	if (patternviewconfig_issinglepatterndisplay(psycleconfig_patview(
			workspace_conf(workspace)))) {
		psy_ui_checkbox_check(&self->displaysinglepattern);
	}
	psy_signal_connect(&self->displaysinglepattern.signal_clicked, self,
		patternviewbar_ondisplaysinglepattern);
	psy_ui_label_init(&self->status, patternviewbar_base(self));
	psy_ui_label_preventtranslation(&self->status);
	psy_ui_label_setcharnumber(&self->status, 40);
	psy_signal_connect(&psycleconfig_patview(
			workspace_conf(workspace))->signal_changed, self,
		patternviewbar_onconfigure);
	psy_signal_connect(&workspace->signal_patterncursorchanged, self,
		patternviewbar_onupdatestatus);
	psy_signal_connect(&workspace->sequenceselection.signal_changed,
		self, patternviewbar_onsequenceselectionchanged);
	psy_signal_connect(&workspace->signal_songchanged, self,
		patternviewbar_onsongchanged);
	if (patternviewconfig_ismovecursorwhenpaste(psycleconfig_patview(
			workspace_conf(workspace)))) {
		psy_ui_checkbox_check(&self->movecursorwhenpaste);
	} else {
		psy_ui_checkbox_disablecheck(&self->movecursorwhenpaste);
	}	
	patternviewbar_updatestatus(self);	
}

void patternviewbar_onmovecursorwhenpaste(PatternViewBar* self, psy_ui_Component* sender)
{
	patternviewconfig_setmovecursorwhenpaste(
		psycleconfig_patview(workspace_conf(self->workspace)),
		psy_ui_checkbox_checked(&self->movecursorwhenpaste));
}

void patternviewbar_ondefaultline(PatternViewBar* self, psy_ui_CheckBox* sender)
{	
	patternviewconfig_togglepatdefaultline(
		psycleconfig_patview(workspace_conf(self->workspace)));
}

void patternviewbar_ondisplaysinglepattern(PatternViewBar* self, psy_ui_CheckBox* sender)
{
	patternviewconfig_setdisplaysinglepattern(
		psycleconfig_patview(workspace_conf(self->workspace)),
		psy_ui_checkbox_checked(&self->displaysinglepattern));
}

void patternviewbar_onsongchanged(PatternViewBar* self, Workspace* sender,
	int flag, psy_audio_Song* song)
{
	patternviewbar_updatestatus(self);
}

void patternviewbar_onupdatestatus(PatternViewBar* self, Workspace* sender)
{
	patternviewbar_updatestatus(self);
}

void patternviewbar_onsequenceselectionchanged(PatternViewBar* self,
	psy_audio_SequenceSelection* sender)
{
	patternviewbar_updatestatus(self);
}

void patternviewbar_updatestatus(PatternViewBar* self)
{
	char text[256];
	psy_audio_PatternCursor cursor;
	uintptr_t patternid;	
	psy_audio_SequenceEntry* entry;
	
	cursor = workspace_patterncursor(self->workspace);
	patternid = psy_INDEX_INVALID;
	if (self->workspace->song) {		
		entry = psy_audio_sequence_entry(
			&self->workspace->song->sequence,
			self->workspace->sequenceselection.editposition);
		if (entry) {
			patternid = entry->patternslot;
		}
	}
	if (patternid == psy_INDEX_INVALID) {
		psy_snprintf(text, 256, "Pat --  Ln --  Trk --  Col --:-- Edit");
	} else {
		psy_snprintf(text, 256, "Pat %d  Ln %d  Trk %d  Col %d:%d Edit",
			patternid, cursor.line, cursor.track, cursor.column, cursor.digit);
	}
	psy_ui_label_settext(&self->status, text);
}

void patternviewbar_onconfigure(PatternViewBar* self, PatternViewConfig* config,
	psy_Property* property)
{
	if (patternviewconfig_ismovecursorwhenpaste(config)) {
		psy_ui_checkbox_check(&self->movecursorwhenpaste);
	} else {
		psy_ui_checkbox_disablecheck(&self->movecursorwhenpaste);
	}
	if (patternviewconfig_issinglepatterndisplay(config)) {
		psy_ui_checkbox_check(&self->displaysinglepattern);
	} else {
		psy_ui_checkbox_disablecheck(&self->displaysinglepattern);
	}
}
