// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternproperties.h"
#include "songio.h"
#include "command.h"
#include <stdlib.h>
#include "../../detail/portable.h"

// Commands
typedef struct {
	psy_Command command;
	psy_audio_Pattern* pattern;
	char* newname;
	char* oldname;
	psy_dsp_big_beat_t newlength;
	psy_dsp_big_beat_t oldlength;
} PatternPropertiesApplyCommand;

static void patternpropertiesapplycommand_dispose(PatternPropertiesApplyCommand*);
static void patternpropertiesapplycommand_execute(PatternPropertiesApplyCommand*);
static void patternpropertiesapplycommand_revert(PatternPropertiesApplyCommand*);

// vtable
static psy_CommandVtable patternpropertiesapplycommand_vtable;
static bool patternpropertiesapplycommand_vtable_initialized = FALSE;

static void patternpropertiesapplycommand_vtable_init(PatternPropertiesApplyCommand* self)
{
	if (!patternpropertiesapplycommand_vtable_initialized) {
		patternpropertiesapplycommand_vtable = *(self->command.vtable);
		patternpropertiesapplycommand_vtable.dispose = (psy_fp_command)
			patternpropertiesapplycommand_dispose;
		patternpropertiesapplycommand_vtable.execute = (psy_fp_command)
			patternpropertiesapplycommand_execute;
		patternpropertiesapplycommand_vtable.revert = (psy_fp_command)
			patternpropertiesapplycommand_revert;
		patternpropertiesapplycommand_vtable_initialized = TRUE;
	}
}

static PatternPropertiesApplyCommand* patternpropertiesapplycommand_allocinit(psy_audio_Pattern* pattern,
	const char* name, psy_dsp_big_beat_t length)
{
	PatternPropertiesApplyCommand* rv;

	rv = malloc(sizeof(PatternPropertiesApplyCommand));
	if (rv) {
		psy_command_init(&rv->command);
		patternpropertiesapplycommand_vtable_init(rv);
		rv->command.vtable = &patternpropertiesapplycommand_vtable;
		rv->pattern = pattern;
		rv->newname = strdup(name);
		rv->newlength = length;
		rv->oldname = 0;
	}
	return rv;
}

void patternpropertiesapplycommand_dispose(PatternPropertiesApplyCommand* self)
{
	free(self->newname);
	self->newname = 0;
	free(self->oldname);
	self->oldname = 0;
}

void patternpropertiesapplycommand_execute(PatternPropertiesApplyCommand* self)
{
	self->oldname = strdup(psy_audio_pattern_name(self->pattern));
	psy_audio_pattern_setname(self->pattern, self->newname);
	free(self->newname);
	self->oldlength = psy_audio_pattern_length(self->pattern);
	psy_audio_pattern_setlength(self->pattern, self->newlength);
	self->newlength = 0.f;
	self->newname = 0;
}

void patternpropertiesapplycommand_revert(PatternPropertiesApplyCommand* self)
{
	self->newname = strdup(psy_audio_pattern_name(self->pattern));
	psy_audio_pattern_setname(self->pattern, self->oldname);
	free(self->oldname);
	self->oldname = 0;
	self->newlength = psy_audio_pattern_length(self->pattern);
	psy_audio_pattern_setlength(self->pattern, self->oldlength);
	self->oldlength = 0.f;
}

static void patternproperties_onsongchanged(PatternProperties*, Workspace*,
	int flag, psy_audio_SongFile*);
static void patternproperties_connectsongsignals(PatternProperties*);
static void patternproperties_onpatternnamechanged(PatternProperties*, psy_audio_Patterns*,
	uintptr_t slot);
static void patternproperties_onpatternlengthchanged(PatternProperties*, psy_audio_Patterns*,
	uintptr_t slot);
static void patternproperties_onapply(PatternProperties*,
	psy_ui_Component* sender);

static void patternproperties_onkeydown(PatternProperties*, psy_ui_KeyEvent*);
static void patternproperties_onkeyup(PatternProperties*, psy_ui_KeyEvent*);

static psy_ui_ComponentVtable patternproperties_vtable;
static bool patternproperties_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* patternproperties_vtable_init(PatternProperties* self)
{
	if (!patternproperties_vtable_initialized) {
		patternproperties_vtable = *(self->component.vtable);
		patternproperties_vtable.onkeydown = (psy_ui_fp_component_onkeydown)
			patternproperties_onkeydown;
		patternproperties_vtable.onkeyup = (psy_ui_fp_component_onkeydown)
			patternproperties_onkeyup;
		patternproperties_vtable_initialized = TRUE;
	}
	return &patternproperties_vtable;
}

void patternproperties_init(PatternProperties* self, psy_ui_Component* parent,
	psy_audio_Pattern* pattern, Workspace* workspace)
{
	self->workspace = workspace;
	self->pattern = pattern;

	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setvtable(&self->component,
		patternproperties_vtable_init(self));
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make(psy_ui_value_makepx(0), psy_ui_value_makeew(2.0),
			psy_ui_value_makeeh(1.0), psy_ui_value_makepx(0)));
	psy_ui_label_init(&self->namelabel, &self->component);
	psy_ui_label_settext(&self->namelabel, "Pattern Name");
	psy_ui_label_settextalignment(&self->namelabel, psy_ui_ALIGNMENT_LEFT);
	psy_ui_edit_init(&self->nameedit, &self->component);
	psy_ui_edit_settext(&self->nameedit, "No Pattern");
	psy_ui_edit_setcharnumber(&self->nameedit, 40);
	psy_ui_label_init(&self->lengthlabel, &self->component);
	psy_ui_label_settext(&self->lengthlabel, "Length");
	psy_ui_label_settextalignment(&self->lengthlabel, psy_ui_ALIGNMENT_LEFT);
	psy_ui_edit_init(&self->lengthedit, &self->component);
	psy_ui_edit_setcharnumber(&self->lengthedit, 20);
	psy_ui_button_init_connect(&self->applybutton, &self->component, self,
		patternproperties_onapply);
	psy_ui_button_settext(&self->applybutton, "Apply");
	psy_ui_button_settextalignment(&self->applybutton, psy_ui_ALIGNMENT_LEFT);
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		patternproperties_onsongchanged);
	patternproperties_connectsongsignals(self);
}

void patternproperties_setpattern(PatternProperties* self,
	psy_audio_Pattern* pattern)
{
	char buffer[20];
	self->pattern = pattern;
	if (self->pattern) {
		psy_ui_edit_settext(&self->nameedit, psy_audio_pattern_name(pattern));
		psy_snprintf(buffer, 20, "%.4f", (float)psy_audio_pattern_length(pattern));
	} else {
		psy_ui_edit_settext(&self->nameedit, "");
		psy_snprintf(buffer, 10, "");
	}
	psy_ui_edit_settext(&self->lengthedit, buffer);
}

void patternproperties_onapply(PatternProperties* self,
	psy_ui_Component* sender)
{
	if (workspace_song(self->workspace) && self->pattern) {
		psy_signal_prevent(&workspace_song(self->workspace)->patterns.signal_namechanged,
			self, patternproperties_onpatternnamechanged);
		psy_signal_prevent(&workspace_song(self->workspace)->patterns.signal_namechanged,
			self, patternproperties_onpatternlengthchanged);
		psy_undoredo_execute(&self->workspace->undoredo,
			&patternpropertiesapplycommand_allocinit(self->pattern,
				psy_ui_edit_text(&self->nameedit),
				(psy_dsp_big_beat_t)atof(psy_ui_edit_text(&self->lengthedit))
			)->command);
		psy_signal_enable(&workspace_song(self->workspace)->patterns.signal_namechanged,
			self, patternproperties_onpatternnamechanged);
		psy_signal_enable(&workspace_song(self->workspace)->patterns.signal_namechanged,
			self, patternproperties_onpatternlengthchanged);
	}
}

void patternproperties_onkeydown(PatternProperties* self, psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_RETURN) {
		patternproperties_onapply(self, &self->component);
		psy_ui_keyevent_preventdefault(ev);
	} else if (ev->keycode == psy_ui_KEY_ESCAPE) {
		workspace_selectview(self->workspace, VIEW_ID_PATTERNVIEW, 0, 0);
		psy_ui_keyevent_preventdefault(ev);
	}
	psy_ui_keyevent_stoppropagation(ev);
}

void patternproperties_onkeyup(PatternProperties* self, psy_ui_KeyEvent* ev)
{
	psy_ui_keyevent_stoppropagation(ev);
}

void patternproperties_onpatternnamechanged(PatternProperties* self,
	psy_audio_Patterns* patterns, uintptr_t slot)
{
	psy_audio_Pattern* pattern;

	pattern = psy_audio_patterns_at(patterns, slot);
	if (pattern && pattern == self->pattern) {
		psy_ui_edit_settext(&self->nameedit,
			psy_audio_pattern_name(pattern));
	}
}

void patternproperties_onpatternlengthchanged(PatternProperties* self,
	psy_audio_Patterns* patterns, uintptr_t slot)
{
	psy_audio_Pattern* pattern;

	pattern = psy_audio_patterns_at(patterns, slot);
	if (pattern && pattern == self->pattern) {
		char buffer[20];

		psy_snprintf(buffer, 20, "%.4f", pattern->length);
		psy_ui_edit_settext(&self->lengthedit, buffer);
	}
}

void patternproperties_onsongchanged(PatternProperties* self, Workspace* workspace, int flag,
	psy_audio_SongFile* songfile)
{
	patternproperties_connectsongsignals(self);
}

void patternproperties_connectsongsignals(PatternProperties* self)
{
	if (workspace_song(self->workspace)) {
		psy_signal_connect(&workspace_song(self->workspace)->patterns.signal_namechanged, self,
			patternproperties_onpatternnamechanged);
		psy_signal_connect(&workspace_song(self->workspace)->patterns.signal_lengthchanged, self,
			patternproperties_onpatternlengthchanged);
	}
}
