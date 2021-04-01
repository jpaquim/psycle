// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "playbar.h"
// host
#include "resources/resource.h"
#include "styles.h"
// audio
#include <exclusivelock.h>
// platform
#include "../../detail/portable.h"

enum {
	PLAY_SONG,
	PLAY_SEL,
	PLAY_BEATS
};

static void playbar_updatetext(PlayBar*);
static void playbar_onloopclicked(PlayBar*, psy_ui_Component* sender);
static void playbar_onrecordnotesclicked(PlayBar*, psy_ui_Component* sender);
static void playbar_onplaymodeselchanged(PlayBar*,
	psy_ui_ComboBox* sender, int sel);
static void playbar_onnumplaybeatsless(PlayBar*, psy_ui_Button* sender);
static void playbar_onnumplaybeatsmore(PlayBar*, psy_ui_Button* sender);
static void playbar_onplayclicked(PlayBar*, psy_ui_Component* sender);
static void playbar_startplay(PlayBar*);
static void playbar_onstopclicked(PlayBar*, psy_ui_Component* sender);
static void playbar_ontimer(PlayBar*, uintptr_t timerid);
static void playbar_onlanguagechanged(PlayBar*);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(PlayBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ontimer = (psy_ui_fp_component_ontimer)playbar_ontimer;
		vtable.onlanguagechanged = (psy_ui_fp_component_onlanguagechanged)
			playbar_onlanguagechanged;
		vtable_initialized = TRUE;
	}
	return &vtable;
}
// implementation
void playbar_init(PlayBar* self, psy_ui_Component* parent, Workspace* workspace)
{				
	psy_ui_Margin spacing;

	psy_ui_margin_init_all_em(&spacing, 0.5, 0.0, 0.5, 1.0);
	psy_ui_component_init(playbar_base(self), parent, NULL);
	psy_ui_component_setvtable(playbar_base(self), vtable_init(self));
	psy_ui_component_setstyletypes(playbar_base(self),
		STYLE_PLAYBAR, psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_INDEX_INVALID);
	psy_ui_component_setspacing(playbar_base(self), &spacing);
	// ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	psy_ui_component_setdefaultalign(playbar_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->workspace = workspace;
	self->player = &workspace->player;
	// loop
	psy_ui_button_init_connect(&self->loop, playbar_base(self), NULL,
		self, playbar_onloopclicked);
	psy_ui_button_settext(&self->loop, "play.loop");
	psy_ui_bitmap_loadresource(&self->loop.bitmapicon, IDB_LOOP_DARK);
	psy_ui_bitmap_settransparency(&self->loop.bitmapicon, psy_ui_colour_make(0x00FFFFFF));
	// record
	psy_ui_button_init_connect(&self->recordnotes, playbar_base(self), NULL,
		self, playbar_onrecordnotesclicked);
	psy_ui_button_settext(&self->recordnotes, "play.record-notes");
	// play
	psy_ui_button_init_connect(&self->play, playbar_base(self), NULL,
		self, playbar_onplayclicked);
	psy_ui_button_settext(&self->play, "play.play");
	psy_ui_bitmap_loadresource(&self->play.bitmapicon, IDB_PLAY_DARK);
	psy_ui_bitmap_settransparency(&self->play.bitmapicon, psy_ui_colour_make(0x00FFFFFF));
	// playmode
	psy_ui_combobox_init(&self->playmode, playbar_base(self), NULL);
	psy_ui_combobox_setcharnumber(&self->playmode, 6);	
	// play beat num
	psy_ui_edit_init(&self->loopbeatsedit, playbar_base(self));
	psy_ui_edit_settext(&self->loopbeatsedit, "4.00");
	psy_ui_edit_setcharnumber(&self->loopbeatsedit, 6);
	psy_ui_button_init_connect(&self->loopbeatsless, playbar_base(self), NULL,
		self, playbar_onnumplaybeatsless);
	psy_ui_button_seticon(&self->loopbeatsless, psy_ui_ICON_LESS);	
	psy_ui_button_init_connect(&self->loopbeatsmore, playbar_base(self), NULL,
		self, playbar_onnumplaybeatsmore);
	psy_ui_button_seticon(&self->loopbeatsmore, psy_ui_ICON_MORE);	
	// stop
	psy_ui_button_init_connect(&self->stop, playbar_base(self), NULL,
		self, playbar_onstopclicked);
	psy_ui_button_settext(&self->stop, "play.stop");
	psy_ui_bitmap_loadresource(&self->stop.bitmapicon, IDB_STOP_DARK);
	psy_ui_bitmap_settransparency(&self->stop.bitmapicon, psy_ui_colour_make(0x00FFFFFF));
	playbar_updatetext(self);
	psy_ui_combobox_setcursel(&self->playmode, 0);
	psy_signal_connect(&self->playmode.signal_selchanged, self,
		playbar_onplaymodeselchanged);	
	psy_ui_component_starttimer(playbar_base(self), 0, 100);
}

void playbar_updatetext(PlayBar* self)
{
	psy_ui_combobox_clear(&self->playmode);
	psy_ui_combobox_addtext(&self->playmode, psy_ui_translate("play.song"));
	psy_ui_combobox_addtext(&self->playmode, psy_ui_translate("play.sel"));
	psy_ui_combobox_addtext(&self->playmode, psy_ui_translate("play.beats"));
}

void playbar_onplaymodeselchanged(PlayBar* self, psy_ui_ComboBox* sender, int sel)
{
	psy_audio_exclusivelock_enter();
	switch (psy_ui_combobox_cursel(&self->playmode)) {
		case PLAY_SONG:
			psy_audio_player_stop(self->player);
			psy_audio_sequencer_setplaymode(&self->player->sequencer,
				psy_audio_SEQUENCERPLAYMODE_PLAYALL);			
		break;
		case PLAY_SEL:
			psy_audio_player_stop(self->player);
			psy_audio_sequencer_setplaymode(&self->player->sequencer,
				psy_audio_SEQUENCERPLAYMODE_PLAYSEL);
			playbar_startplay(self);
		break;
		case PLAY_BEATS:
			psy_audio_player_stop(self->player);
			psy_audio_sequencer_setplaymode(&self->player->sequencer,
				psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS);
			playbar_startplay(self);
		break;
		default:
			psy_audio_player_stop(self->player);
			psy_audio_sequencer_setplaymode(&self->player->sequencer,
				psy_audio_SEQUENCERPLAYMODE_PLAYALL);
			playbar_startplay(self);
		break;
	}
	psy_audio_exclusivelock_leave();
}

void playbar_onnumplaybeatsless(PlayBar* self, psy_ui_Button* sender)
{
	psy_dsp_big_beat_t playbeats;
	char text[40];
	
	playbeats = (psy_dsp_big_beat_t) atof(psy_ui_edit_text(&self->loopbeatsedit));
	if (playbeats > 1) {
		playbeats -= 1;
	}
	psy_audio_sequencer_setnumplaybeats(&self->player->sequencer, playbeats);
	psy_snprintf(text, 40, "%f", (double) playbeats);
	psy_ui_edit_settext(&self->loopbeatsedit, text);
}

void playbar_onnumplaybeatsmore(PlayBar* self, psy_ui_Button* sender)
{
	psy_dsp_big_beat_t playbeats;
	char text[40];
	
	playbeats = (psy_dsp_big_beat_t) atof(psy_ui_edit_text(&self->loopbeatsedit));	
	playbeats += 1;		
	psy_audio_sequencer_setnumplaybeats(&self->player->sequencer, playbeats);
	psy_snprintf(text, 40, "%f", (double) playbeats);
	psy_ui_edit_settext(&self->loopbeatsedit, text);
}

void playbar_onplayclicked(PlayBar* self, psy_ui_Component* sender)
{
	switch (psy_ui_combobox_cursel(&self->playmode)) {
		case PLAY_SONG:
			psy_audio_sequencer_setplaymode(&self->player->sequencer,
				psy_audio_SEQUENCERPLAYMODE_PLAYALL);
		break;
		case PLAY_SEL:
			psy_audio_sequencer_setplaymode(&self->player->sequencer,
				psy_audio_SEQUENCERPLAYMODE_PLAYSEL);
		break;
		case PLAY_BEATS:
			psy_audio_sequencer_setplaymode(&self->player->sequencer,
				psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS);
		break;
		default:
			psy_audio_sequencer_setplaymode(&self->player->sequencer,
				psy_audio_SEQUENCERPLAYMODE_PLAYALL);
		break;
	};	
	if (!psy_audio_player_playing(self->player)) {
		playbar_startplay(self);
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
		self->workspace->sequenceselection.editposition);
	if (entry) {
		psy_audio_exclusivelock_enter();		
		psy_audio_player_stop(self->player);
		psy_audio_sequence_setplayselection(sequence,
			&self->workspace->sequenceselection);
		startposition = entry->offset;
		if (psy_audio_sequencer_playmode(&self->player->sequencer)
			== psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS) {
			psy_audio_PatternCursor editposition;

			editposition = workspace_patterncursor(self->workspace);			
			startposition += (psy_dsp_big_beat_t) editposition.offset;
			numplaybeats = (psy_dsp_big_beat_t) atof(psy_ui_edit_text(
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
		psy_audio_sequencer_stoploop(&self->player->sequencer);
		psy_ui_button_disablehighlight(&self->loop);
	} else {
		psy_audio_sequencer_loop(&self->player->sequencer);
		psy_ui_button_highlight(&self->loop);
	}
}

void playbar_onrecordnotesclicked(PlayBar* self, psy_ui_Component* sender)
{	
	if (psy_audio_player_recordingnotes(self->player)) {
		psy_audio_player_stoprecordingnotes(self->player);
		psy_ui_button_disablehighlight(&self->recordnotes);
	} else {
		psy_audio_player_startrecordingnotes(self->player);
		psy_ui_button_highlight(&self->recordnotes);
	}
}

void playbar_ontimer(PlayBar* self, uintptr_t timerid)
{	
	if (psy_audio_player_playing(self->player)) {
		psy_ui_button_highlight(&self->play);
	} else {
		psy_ui_button_disablehighlight(&self->play);
	}
	if (psy_audio_sequencer_looping(&self->player->sequencer)) {
		psy_ui_button_highlight(&self->loop);
	} else {
		psy_ui_button_disablehighlight(&self->loop);
	}
}

void playbar_onlanguagechanged(PlayBar* self)
{
	playbar_updatetext(self);
}
