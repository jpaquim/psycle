/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternviewbar.h"
/* local */
#include "patternview.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void patternviewbar_on_move_cursor_when_paste(PatternViewBar*,
	psy_ui_Component* sender);
static void patternviewbar_on_default_line(PatternViewBar*,
	psy_ui_CheckBox* sender);
static void patternviewbar_on_display_single_pattern(PatternViewBar*,
	psy_ui_CheckBox* sender);
static void patternviewbar_on_cursor_changed(PatternViewBar*,
	psy_audio_Sequence* sender);
static void patternviewbar_on_playline_changed(PatternViewBar*, Workspace*
	sender);
static void patternviewbar_on_playstatus_changed(PatternViewBar*, Workspace*
	sender);
static void patternviewbar_on_song_changed(PatternViewBar*, Workspace* sender);
static void patternviewbar_connect_song(PatternViewBar*);
static void patternviewbar_update_status(PatternViewBar*);
static void patternviewbar_status_text(PatternViewBar*, uintptr_t maxcount,
	char* rv);
static void patternviewbar_on_zoombox_changed(PatternViewBar*, ZoomBox* sender);
static void patternviewbar_on_configure(PatternViewBar*, PatternViewConfig*,
	psy_Property*);

/* implementation */
void patternviewbar_init(PatternViewBar* self, psy_ui_Component* parent,
	PatternView* patternview, Workspace* workspace)
{		
	assert(self);
	assert(workspace);

	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	self->patternview = patternview;	
	psy_ui_component_setdefaultalign(patternviewbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	/* Zoom */
	zoombox_init(&self->zoombox, patternviewbar_base(self));
	psy_ui_component_setpreferredsize(&self->zoombox.component,
		psy_ui_size_make_em(16.0, 1.0));
	psy_signal_connect(&self->zoombox.signal_changed, self,
		patternviewbar_on_zoombox_changed);
	patterncursorstepbox_init(&self->cursorstep, &self->component, workspace);	
	/* Move cursor when paste */
	psy_ui_checkbox_init(&self->movecursorwhenpaste, patternviewbar_base(self));
	psy_ui_checkbox_settext(&self->movecursorwhenpaste,
		"settingsview.pv.move-cursor-when-paste");	
	psy_signal_connect(&self->movecursorwhenpaste.signal_clicked, self,
		patternviewbar_on_move_cursor_when_paste);
	/* Default line */
	psy_ui_checkbox_init(&self->defaultentries, patternviewbar_base(self));
	psy_ui_checkbox_settext(&self->defaultentries,
		"settingsview.visual.default-line");
	if (patternviewconfig_defaultline(psycleconfig_patview(
			workspace_conf(workspace)))) {
		psy_ui_checkbox_check(&self->defaultentries);
	}
	psy_signal_connect(&self->defaultentries.signal_clicked, self,
		patternviewbar_on_default_line);
	/* Single pattern display mode */
	psy_ui_checkbox_init(&self->displaysinglepattern,
		patternviewbar_base(self));
	psy_ui_checkbox_settext(&self->displaysinglepattern,
		"settingsview.pv.displaysinglepattern");
	if (patternviewconfig_issinglepatterndisplay(psycleconfig_patview(
			workspace_conf(workspace)))) {
		psy_ui_checkbox_check(&self->displaysinglepattern);
	}
	psy_signal_connect(&self->displaysinglepattern.signal_clicked, self,
		patternviewbar_on_display_single_pattern);
	psy_ui_label_init(&self->status, patternviewbar_base(self));
	psy_ui_label_preventtranslation(&self->status);	
	psy_ui_label_setcharnumber(&self->status, 30.0);
	psy_signal_connect(&psycleconfig_patview(
		workspace_conf(workspace))->signal_changed, self,
		patternviewbar_on_configure);
	psy_signal_connect(&workspace->signal_songchanged, self,
		patternviewbar_on_song_changed);
	if (patternviewconfig_ismovecursorwhenpaste(psycleconfig_patview(
			workspace_conf(workspace)))) {
		psy_ui_checkbox_check(&self->movecursorwhenpaste);
	} else {
		psy_ui_checkbox_disablecheck(&self->movecursorwhenpaste);
	}	
	patternviewbar_update_status(self);
	patternviewbar_connect_song(self);
	psy_signal_connect(&self->workspace->signal_playlinechanged, self,
		patternviewbar_on_playline_changed);
	psy_signal_connect(&self->workspace->signal_playstatuschanged, self,
		patternviewbar_on_playstatus_changed);
}

void patternviewbar_on_move_cursor_when_paste(PatternViewBar* self,
	psy_ui_Component* sender)
{
	assert(self);

	patternviewconfig_setmovecursorwhenpaste(
		psycleconfig_patview(workspace_conf(self->workspace)),
		psy_ui_checkbox_checked(&self->movecursorwhenpaste));
}

void patternviewbar_on_default_line(PatternViewBar* self, psy_ui_CheckBox*
	sender)
{	
	assert(self);

	patternviewconfig_togglepatdefaultline(
		psycleconfig_patview(workspace_conf(self->workspace)));
}

void patternviewbar_on_display_single_pattern(PatternViewBar* self,
	psy_ui_CheckBox* sender)
{
	assert(self);

	patternviewconfig_setdisplaysinglepattern(
		psycleconfig_patview(workspace_conf(self->workspace)),
		psy_ui_checkbox_checked(&self->displaysinglepattern));
}

void patternviewbar_on_song_changed(PatternViewBar* self, Workspace* sender)
{
	assert(self);

	patternviewbar_connect_song(self);
	patternviewbar_update_status(self);
}

void patternviewbar_connect_song(PatternViewBar* self)
{
	assert(self);
	assert(self->workspace);

	if (workspace_song(self->workspace)) {
		psy_signal_connect(
			&workspace_song(self->workspace)->sequence.signal_cursorchanged,
			self, patternviewbar_on_cursor_changed);
	}
}

void patternviewbar_on_cursor_changed(PatternViewBar* self,
	psy_audio_Sequence* sender)
{
	assert(self);

	patternviewbar_update_status(self);	
}

void patternviewbar_on_playline_changed(PatternViewBar* self, Workspace* sender)
{		
	if (!workspace_followingsong(sender)) {
		patternviewbar_update_status(self);
	}
}

void patternviewbar_on_playstatus_changed(PatternViewBar* self, Workspace* sender)
{
	patternviewbar_update_status(self);
}

void patternviewbar_update_status(PatternViewBar* self)
{	
	char text[256];

	assert(self);

	patternviewbar_status_text(self, 256, text);
	psy_ui_label_settext(&self->status, text);
}

void patternviewbar_status_text(PatternViewBar* self, uintptr_t maxcount,
	char* rv)
{	
	psy_audio_SequenceCursor cursor;
	uintptr_t patternid;

	assert(self);
	assert(self->workspace);
	
	patternid = psy_INDEX_INVALID;
	if (workspace_song(self->workspace)) {
		cursor = self->workspace->song->sequence.cursor;
		patternid = psy_audio_sequencecursor_patternid(&cursor,
			psy_audio_song_sequence(workspace_song(self->workspace)));
	}
	if (patternid == psy_INDEX_INVALID || !workspace_song(self->workspace)) {
		psy_snprintf(rv, maxcount, "Pat -- Ln -- Trk -- Col --:-- Edt");
	} else if (psy_audio_player_playing(&self->workspace->player)) {		
		psy_snprintf(rv, maxcount, "Pat %d Ln %d Trk %d Col %d:%d Edt",
			(int)self->workspace->currplaycursor.patternid,
			(int)self->workspace->currplayline,
			(int)psy_audio_sequencecursor_track(&cursor),
			(int)psy_audio_sequencecursor_column(&cursor),
			(int)psy_audio_sequencecursor_digit(&cursor));		
	} else {
		psy_snprintf(rv, maxcount, "Pat %d Ln %d Trk %d Col %d:%d Edt",
			(int)patternid,
			(int)psy_audio_sequencecursor_line(&cursor),
			(int)psy_audio_sequencecursor_track(&cursor),
			(int)psy_audio_sequencecursor_column(&cursor),
			(int)psy_audio_sequencecursor_digit(&cursor));
	}
}

void patternviewbar_on_configure(PatternViewBar* self, PatternViewConfig* config,
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

void patternviewbar_on_zoombox_changed(PatternViewBar* self, ZoomBox* sender)
{
	assert(self);

	self->patternview->zoom = zoombox_rate(&self->zoombox);
	patternview_updatefont(self->patternview);
}
