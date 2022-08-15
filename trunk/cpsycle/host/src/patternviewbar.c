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


/* PatternViewStatus */

/* implementation */
void patternviewstatus_init(PatternViewStatus* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make_em(0.0, 0.5, 0.0, 0.0));
	/* pattern index */
	psy_ui_label_init(&self->pat_desc, &self->component);
	psy_ui_label_prevent_translation(&self->pat_desc);
	psy_ui_label_set_text(&self->pat_desc, "Pat");
	psy_ui_label_init(&self->pat, &self->component);
	psy_ui_label_prevent_translation(&self->pat);
	psy_ui_label_set_char_number(&self->pat, 3.0);
	/* line */
	psy_ui_label_init(&self->ln_desc, &self->component);
	psy_ui_label_prevent_translation(&self->ln_desc);
	psy_ui_label_set_text(&self->ln_desc, "Ln");
	psy_ui_label_init(&self->ln, &self->component);
	psy_ui_label_prevent_translation(&self->ln);
	psy_ui_label_set_char_number(&self->ln, 4.0);
	/* track */
	psy_ui_label_init(&self->trk_desc, &self->component);
	psy_ui_label_prevent_translation(&self->trk_desc);
	psy_ui_label_set_text(&self->trk_desc, "Trk");
	psy_ui_label_init(&self->trk, &self->component);
	psy_ui_label_prevent_translation(&self->trk);
	psy_ui_label_set_char_number(&self->trk, 2.0);
	/* col */
	psy_ui_label_init(&self->col_desc, &self->component);
	psy_ui_label_prevent_translation(&self->col_desc);
	psy_ui_label_set_text(&self->col_desc, "Col");
	psy_ui_label_init(&self->col, &self->component);
	psy_ui_label_prevent_translation(&self->col);
	psy_ui_label_set_char_number(&self->col, 4.0);
	/* edit mode */	
	psy_ui_label_init(&self->mode, &self->component);
	psy_ui_label_prevent_translation(&self->mode);
	psy_ui_label_set_char_number(&self->mode, 4.0);
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
		patternid = psy_audio_sequencecursor_pattern_id(&cursor,
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
			patternid = psy_audio_sequencecursor_pattern_id(
				&self->workspace->player.sequencer.hostseqtime.currplaycursor,
				psy_audio_song_sequence(workspace_song(self->workspace)));
			line = psy_audio_sequencecursor_line(&self->workspace->player.sequencer.hostseqtime.currplaycursor);
			// line -= psy_audio_sequencecursor_seqline(&self->workspace->player.sequencer.hostseqtime.currplaycursor);
		} else {
			line = psy_audio_sequencecursor_line(&cursor);
			// line -= psy_audio_sequencecursor_seqline(&cursor);
		}
		psy_snprintf(text, 256, "%d", (int)patternid);		
		psy_ui_label_set_text(&self->pat, text);
		psy_snprintf(text, 256, "%d", (int)line);		
		psy_ui_label_set_text(&self->ln, text);
		psy_snprintf(text, 256, "%d",
			(int)psy_audio_sequencecursor_track(&cursor));
		psy_ui_label_set_text(&self->trk, text);
		psy_snprintf(text, 256, "%d: %d",
			(int)psy_audio_sequencecursor_column(&cursor),
			(int)psy_audio_sequencecursor_digit(&cursor));
		psy_ui_label_set_text(&self->col, text);
		psy_ui_label_set_text(&self->mode, "Edt");		
	}
}

/* PatternViewBar */

/* prototypes */
static void patternviewbar_on_cursor_changed(PatternViewBar*,
	psy_audio_Sequence* sender);
static void patternviewbar_on_playline_changed(PatternViewBar*, Workspace*
	sender);
static void patternviewbar_on_playstatus_changed(PatternViewBar*, Workspace*
	sender);
static void patternviewbar_on_song_changed(PatternViewBar*, Workspace* sender);
static void patternviewbar_connect_song(PatternViewBar*);
static void patternviewbar_update_status(PatternViewBar*);

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
	psy_ui_component_set_default_align(patternviewbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));	
	/* Zoom */
	zoombox_init(&self->zoombox, patternviewbar_base(self));		
	zoombox_data_exchange(&self->zoombox, patternviewconfig_property(
		self->patconfig, "zoom"));	
	patterncursorstepbox_init(&self->cursorstep, &self->component, workspace);	
	/* Move cursor when paste */
	psy_ui_checkbox_init(&self->movecursorwhenpaste, patternviewbar_base(self));
	psy_ui_checkbox_prevent_wrap(&self->movecursorwhenpaste);
	psy_ui_checkbox_set_text(&self->movecursorwhenpaste,
		"settings.pv.move-cursor-when-paste");	
	psy_ui_checkbox_data_exchange(&self->movecursorwhenpaste,
		patternviewconfig_property(psycleconfig_patview(
			workspace_conf(workspace)), 
			"movecursorwhenpaste"));
	/* Default line */
	psy_ui_checkbox_init(&self->defaultentries, patternviewbar_base(self));
	psy_ui_checkbox_prevent_wrap(&self->defaultentries);
	psy_ui_checkbox_set_text(&self->defaultentries,
		"settings.visual.default-line");
	psy_ui_checkbox_data_exchange(&self->defaultentries,
		patternviewconfig_property(psycleconfig_patview(
			workspace_conf(workspace)), 
			"griddefaults"));	
	/* Single pattern display mode */
	psy_ui_checkbox_init(&self->displaysinglepattern,
		patternviewbar_base(self));
	psy_ui_checkbox_prevent_wrap(&self->displaysinglepattern);
	psy_ui_checkbox_set_text(&self->displaysinglepattern,
		"settings.pv.displaysinglepattern");
	psy_ui_checkbox_data_exchange(&self->displaysinglepattern,
		patternviewconfig_property(psycleconfig_patview(
			workspace_conf(workspace)), 
			"displaysinglepattern"));
	patternviewstatus_init(&self->status, patternviewbar_base(self), workspace);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		patternviewbar_on_song_changed);	
	patternviewbar_update_status(self);
	patternviewbar_connect_song(self);
	psy_signal_connect(&self->workspace->player.sequencer.signal_play_line_changed, self,
		patternviewbar_on_playline_changed);
	psy_signal_connect(&self->workspace->player.sequencer.signal_play_status_changed, self,
		patternviewbar_on_playstatus_changed);
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
