/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "playbar.h"
/* host */
#include "resources/resource.h"
#include "styles.h"
/* audio */
#include <exclusivelock.h>
/* platform */
#include "../../detail/portable.h"

#define PLAYBAR_TIMERINTERVAL 100

/* combobox index */
enum {
	PLAY_SONG  = 0,
	PLAY_SEL   = 1,
	PLAY_BEATS = 2
};

/* prototypes */
static void playbar_updatetext(PlayBar*);
static void playbar_onloopclicked(PlayBar*, psy_ui_Component* sender);
static void playbar_onrecordnotesclicked(PlayBar*, psy_ui_Component* sender);
static void playbar_onplaymodeselchanged(PlayBar*, psy_ui_ComboBox* sender,
	int sel);
static void playbar_onnumplaybeatsless(PlayBar*, psy_ui_Button* sender);
static void playbar_onnumplaybeatsmore(PlayBar*, psy_ui_Button* sender);
static void playbar_onplayclicked(PlayBar*, psy_ui_Component* sender);
static void playbar_startplay(PlayBar*);
static void playbar_onstopclicked(PlayBar*, psy_ui_Component* sender);
static void playbar_on_timer(PlayBar*, uintptr_t timerid);
static void playbar_onlanguagechanged(PlayBar*);
static psy_audio_SequencerPlayMode playbar_comboboxplaymode(const PlayBar*);
/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(PlayBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			playbar_on_timer;
		vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			playbar_onlanguagechanged;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(playbar_base(self), &vtable);	
}
/* implementation */
void playbar_init(PlayBar* self, psy_ui_Component* parent, Workspace* workspace)
{					
	psy_ui_component_init(playbar_base(self), parent, NULL);
	vtable_init(self);
	psy_ui_component_set_style_type(playbar_base(self), STYLE_PLAYBAR);	
	psy_ui_component_init(&self->buttons, playbar_base(self), NULL);
	psy_ui_component_set_align(&self->buttons, psy_ui_ALIGN_TOP);
	psy_ui_component_set_default_align(&self->buttons, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_component_set_minimum_size(playbar_base(self), 
		psy_ui_size_make_em(30.0, 0.0));
	self->workspace = workspace;
	self->player = workspace_player(workspace);
	/* loop */
	psy_ui_button_init_text_connect(&self->loop, &self->buttons,
		"play.loop", self, playbar_onloopclicked);	
	psy_ui_button_load_resource(&self->loop, IDB_LOOP_LIGHT, IDB_LOOP_DARK,
		psy_ui_colour_white());
	/* record */
	psy_ui_button_init_text_connect(&self->recordnotes, &self->buttons,
		"play.record-notes", self, playbar_onrecordnotesclicked);
	/* play */
	psy_ui_button_init_text_connect(&self->play, &self->buttons,
		"play.play", self, playbar_onplayclicked);
	psy_ui_button_load_resource(&self->play, IDB_PLAY_LIGHT, IDB_PLAY_DARK,
		psy_ui_colour_white());
	/* playmode */
	psy_ui_combobox_init(&self->playmode, &self->buttons);
	psy_ui_combobox_set_char_number(&self->playmode, 8.0);	
	/* play beat num */
	psy_ui_textarea_init_single_line(&self->loopbeatsedit, &self->buttons);	
	psy_ui_textarea_set_text(&self->loopbeatsedit, "4.00");
	psy_ui_textarea_set_char_number(&self->loopbeatsedit, 6);
	psy_ui_button_init_connect(&self->loopbeatsless, &self->buttons,
		self, playbar_onnumplaybeatsless);
	psy_ui_button_set_icon(&self->loopbeatsless, psy_ui_ICON_LESS);	
	psy_ui_button_init_connect(&self->loopbeatsmore, &self->buttons,
		self, playbar_onnumplaybeatsmore);
	psy_ui_button_set_icon(&self->loopbeatsmore, psy_ui_ICON_MORE);	
	/* stop */
	psy_ui_button_init_text_connect(&self->stop, &self->buttons,
		"play.stop", self, playbar_onstopclicked);	
	psy_ui_button_load_resource(&self->stop, IDB_STOP_LIGHT, IDB_STOP_DARK,
		psy_ui_colour_white());
	playbar_updatetext(self);
	psy_ui_combobox_select(&self->playmode, 0);
	psy_signal_connect(&self->playmode.signal_selchanged, self,
		playbar_onplaymodeselchanged);	
	psy_ui_component_start_timer(playbar_base(self), 0, PLAYBAR_TIMERINTERVAL);	
}

void playbar_updatetext(PlayBar* self)
{
	psy_ui_combobox_clear(&self->playmode);
	psy_ui_combobox_add_text(&self->playmode, psy_ui_translate("play.song"));
	psy_ui_combobox_add_text(&self->playmode, psy_ui_translate("play.sel"));
	psy_ui_combobox_add_text(&self->playmode, psy_ui_translate("play.beats"));
}

void playbar_onplaymodeselchanged(PlayBar* self, psy_ui_ComboBox* sender, int sel)
{
	psy_audio_exclusivelock_enter();
	psy_audio_player_stop(self->player);
	psy_audio_sequencer_set_play_mode(&self->player->sequencer,
		playbar_comboboxplaymode(self));
	playbar_startplay(self);	
	psy_audio_exclusivelock_leave();
}

void playbar_onnumplaybeatsless(PlayBar* self, psy_ui_Button* sender)
{
	psy_dsp_big_beat_t playbeats;
	char text[40];
	
	playbeats = (psy_dsp_big_beat_t) atof(psy_ui_textarea_text(&self->loopbeatsedit));
	if (playbeats > 1) {
		playbeats -= 1;
	}
	psy_audio_sequencer_set_num_play_beats(&self->player->sequencer, playbeats);
	psy_snprintf(text, 40, "%f", (double) playbeats);
	psy_ui_textarea_set_text(&self->loopbeatsedit, text);
}

void playbar_onnumplaybeatsmore(PlayBar* self, psy_ui_Button* sender)
{
	psy_dsp_big_beat_t playbeats;
	char text[40];
	
	playbeats = (psy_dsp_big_beat_t) atof(psy_ui_textarea_text(&self->loopbeatsedit));
	playbeats += 1;		
	psy_audio_sequencer_set_num_play_beats(&self->player->sequencer, playbeats);
	psy_snprintf(text, 40, "%f", (double) playbeats);
	psy_ui_textarea_set_text(&self->loopbeatsedit, text);
}

void playbar_onplayclicked(PlayBar* self, psy_ui_Component* sender)
{
	psy_audio_sequencer_set_play_mode(&self->player->sequencer,
		playbar_comboboxplaymode(self));	
	playbar_startplay(self);	
}

psy_audio_SequencerPlayMode playbar_comboboxplaymode(const PlayBar* self)
{
	switch (psy_ui_combobox_cursel(&self->playmode)) {
	case PLAY_SONG:
		return psy_audio_SEQUENCERPLAYMODE_PLAYALL;		
	case PLAY_SEL:
		return psy_audio_SEQUENCERPLAYMODE_PLAYSEL;		
	case PLAY_BEATS:
		return psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS;		
	default:
		return psy_audio_SEQUENCERPLAYMODE_PLAYALL;
	}	
}

void playbar_startplay(PlayBar* self)
{
	psy_audio_Sequence* sequence;	
	psy_audio_SequenceEntry* entry;
	psy_dsp_big_beat_t startposition;
	psy_dsp_big_beat_t numplaybeats;
	
	psy_ui_button_highlight(&self->play);
	sequence = self->player->sequencer.sequence;	
	entry = psy_audio_sequence_entry(sequence,
		psy_audio_sequenceselection_first(&self->workspace->song->sequence.selection));
	if (entry) {
		psy_audio_exclusivelock_enter();		
		psy_audio_player_stop(self->player);
		psy_audio_sequence_setplayselection(sequence,
			&self->workspace->song->sequence.selection);
		startposition = psy_audio_sequenceentry_offset(entry);
		if (psy_audio_sequencer_play_mode(&self->player->sequencer)
				== psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS) {
			psy_audio_SequenceCursor editposition;

			editposition = self->workspace->song->sequence.cursor;
			startposition += (psy_dsp_big_beat_t)editposition.offset;
			numplaybeats = (psy_dsp_big_beat_t)atof(psy_ui_textarea_text(
				&self->loopbeatsedit));
			self->player->sequencer.numplaybeats = numplaybeats;
		}
		psy_audio_player_setposition(self->player, startposition);
		psy_audio_player_start(self->player);
		psy_audio_exclusivelock_leave();
	}
}

void playbar_onstopclicked(PlayBar* self, psy_ui_Component* sender)
{
	psy_audio_player_stop(self->player);
}

void playbar_onloopclicked(PlayBar* self, psy_ui_Component* sender)
{
	if (psy_audio_sequencer_looping(&self->player->sequencer)) {
		psy_audio_sequencer_stop_loop(&self->player->sequencer);
		psy_ui_button_disable_highlight(&self->loop);
	} else {
		psy_audio_sequencer_loop(&self->player->sequencer);
		psy_ui_button_highlight(&self->loop);
	}
}

void playbar_onrecordnotesclicked(PlayBar* self, psy_ui_Component* sender)
{	
	if (psy_audio_player_recordingnotes(self->player)) {
		psy_audio_player_stoprecordingnotes(self->player);
		psy_ui_button_disable_highlight(&self->recordnotes);
	} else {
		psy_audio_player_startrecordingnotes(self->player);
		psy_ui_button_highlight(&self->recordnotes);
	}
}

void playbar_on_timer(PlayBar* self, uintptr_t timerid)
{	
	if (psy_audio_player_playing(self->player)) {
		psy_ui_button_highlight(&self->play);
	} else {
		psy_ui_button_disable_highlight(&self->play);
	}
	if (psy_audio_sequencer_looping(&self->player->sequencer)) {
		psy_ui_button_highlight(&self->loop);
	} else {
		psy_ui_button_disable_highlight(&self->loop);
	}
}

void playbar_onlanguagechanged(PlayBar* self)
{
	playbar_updatetext(self);
}
