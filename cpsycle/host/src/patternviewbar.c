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

void patternviewstatus_init(PatternViewStatus* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	psy_ui_component_set_defaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));
	/* pattern index */
	psy_ui_label_init(&self->pat_desc, &self->component);
	psy_ui_label_prevent_translation(&self->pat_desc);
	psy_ui_label_set_text(&self->pat_desc, "Pat");
	psy_ui_label_init(&self->pat, &self->component);
	psy_ui_label_prevent_translation(&self->pat);
	psy_ui_label_set_charnumber(&self->pat, 3.0);
	/* line */
	psy_ui_label_init(&self->ln_desc, &self->component);
	psy_ui_label_prevent_translation(&self->ln_desc);
	psy_ui_label_set_text(&self->ln_desc, "Ln");
	psy_ui_label_init(&self->ln, &self->component);
	psy_ui_label_prevent_translation(&self->ln);
	psy_ui_label_set_charnumber(&self->ln, 3.0);
	/* track */
	psy_ui_label_init(&self->trk_desc, &self->component);
	psy_ui_label_prevent_translation(&self->trk_desc);
	psy_ui_label_set_text(&self->trk_desc, "Trk");
	psy_ui_label_init(&self->trk, &self->component);
	psy_ui_label_prevent_translation(&self->trk);
	psy_ui_label_set_charnumber(&self->trk, 2.0);
	/* col */
	psy_ui_label_init(&self->col_desc, &self->component);
	psy_ui_label_prevent_translation(&self->col_desc);
	psy_ui_label_set_text(&self->col_desc, "Col");
	psy_ui_label_init(&self->col, &self->component);
	psy_ui_label_prevent_translation(&self->col);
	psy_ui_label_set_charnumber(&self->col, 4.0);
	/* edit mode */	
	psy_ui_label_init(&self->mode, &self->component);
	psy_ui_label_prevent_translation(&self->mode);
	psy_ui_label_set_charnumber(&self->mode, 4.0);
}

void patternviewstatus_update(PatternViewStatus* self)
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
		psy_ui_label_set_text(&self->pat, "--");
		psy_ui_label_set_text(&self->ln, "--");
		psy_ui_label_set_text(&self->trk, "--");
		psy_ui_label_set_text(&self->col, "--");
		psy_ui_label_set_text(&self->mode, "Edt");		
	} else {
		char text[256];
		uintptr_t line;

		if (psy_audio_player_playing(&self->workspace->player)) {
			patternid = psy_audio_sequencecursor_patternid(
				&self->workspace->host_sequencer_time.currplaycursor,
				psy_audio_song_sequence(workspace_song(self->workspace)));
			line = psy_audio_sequencecursor_line(&self->workspace->host_sequencer_time.currplaycursor);
			line -= psy_audio_sequencecursor_seqline(&self->workspace->host_sequencer_time.currplaycursor);
		} else {
			line = psy_audio_sequencecursor_line(&cursor);
			line -= psy_audio_sequencecursor_seqline(&cursor);
		}
		itoa((int)patternid, text, 10);
		psy_ui_label_set_text(&self->pat, text);
		itoa((int)line, text, 10);
		psy_ui_label_set_text(&self->ln, text);
		itoa((int)psy_audio_sequencecursor_track(&cursor), text, 10);
		psy_ui_label_set_text(&self->trk, text);
		psy_snprintf(text, 256, "%d: %d",
			(int)psy_audio_sequencecursor_column(&cursor),
			(int)psy_audio_sequencecursor_digit(&cursor));
		psy_ui_label_set_text(&self->col, text);
		psy_ui_label_set_text(&self->mode, "Edt");		
	}
}

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
static void patternviewbar_on_zoombox_changed(PatternViewBar*, ZoomBox* sender);
static void patternviewbar_on_configure(PatternViewBar*, PatternViewConfig*,
	psy_Property*);


/* PatternViewBar */

/* implementation */
void patternviewbar_init(PatternViewBar* self, psy_ui_Component* parent,
	PatternViewConfig* patconfig, Workspace* workspace)
{		
	assert(self);
	assert(patconfig);
	assert(workspace);

	psy_ui_component_init(&self->component, parent, NULL);	
	self->patconfig = patconfig;
	self->workspace = workspace;	
	psy_ui_component_set_defaultalign(patternviewbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));	
	/* Zoom */
	zoombox_init(&self->zoombox, patternviewbar_base(self));
	psy_ui_component_set_preferred_size(&self->zoombox.component,
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
	if (patternviewconfig_single_mode(psycleconfig_patview(
			workspace_conf(workspace)))) {
		psy_ui_checkbox_check(&self->displaysinglepattern);
	}
	psy_signal_connect(&self->displaysinglepattern.signal_clicked, self,
		patternviewbar_on_display_single_pattern);
	patternviewstatus_init(&self->status, patternviewbar_base(self), workspace);	
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
	psy_signal_connect(&self->workspace->signal_play_line_changed, self,
		patternviewbar_on_playline_changed);
	psy_signal_connect(&self->workspace->signal_play_status_changed, self,
		patternviewbar_on_playstatus_changed);
}

void patternviewbar_on_move_cursor_when_paste(PatternViewBar* self,
	psy_ui_Component* sender)
{
	assert(self);

	patternviewconfig_setmovecursorwhenpaste(self->patconfig,
		psy_ui_checkbox_checked(&self->movecursorwhenpaste));
}

void patternviewbar_on_default_line(PatternViewBar* self, psy_ui_CheckBox*
	sender)
{	
	assert(self);

	patternviewconfig_toggle_pattern_defaultline(self->patconfig);
}

void patternviewbar_on_display_single_pattern(PatternViewBar* self,
	psy_ui_CheckBox* sender)
{
	assert(self);

	if (psy_ui_checkbox_checked(&self->displaysinglepattern)) {
		patternviewconfig_display_single_pattern(self->patconfig);
	} else {
		patternviewconfig_display_sequence(self->patconfig);
	}
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
	if (!keyboardmiscconfig_following_song(&sender->config.misc)) {
		patternviewbar_update_status(self);
	}
}

void patternviewbar_on_playstatus_changed(PatternViewBar* self, Workspace* sender)
{
	patternviewbar_update_status(self);
}

void patternviewbar_update_status(PatternViewBar* self)
{
	assert(self);
	
	patternviewstatus_update(&self->status);
}

void patternviewbar_on_configure(PatternViewBar* self, PatternViewConfig* config,
	psy_Property* property)
{
	if (patternviewconfig_ismovecursorwhenpaste(config)) {
		psy_ui_checkbox_check(&self->movecursorwhenpaste);
	} else {
		psy_ui_checkbox_disablecheck(&self->movecursorwhenpaste);
	}
	if (patternviewconfig_single_mode(config)) {
		psy_ui_checkbox_check(&self->displaysinglepattern);
	} else {
		psy_ui_checkbox_disablecheck(&self->displaysinglepattern);
	}
}

void patternviewbar_on_zoombox_changed(PatternViewBar* self, ZoomBox* sender)
{
	assert(self);

	patternviewconfig_set_zoom(self->patconfig, zoombox_rate(sender));
}
