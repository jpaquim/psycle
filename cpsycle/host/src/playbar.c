// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "playbar.h"
#include <exclusivelock.h>
#include <stdlib.h>
#include "../../detail/portable.h"

#define TIMERID_PLAYBAR 400

enum {
	PLAY_SONG,
	PLAY_SEL,
	PLAY_BEATS
};

static void playbar_updatetext(PlayBar*);
static void playbar_initalign(PlayBar*);
static void playbar_onloopclicked(PlayBar*, psy_ui_Component* sender);
static void playbar_onrecordnotesclicked(PlayBar*, psy_ui_Component* sender);
static void playbar_onplaymodeselchanged(PlayBar*,
	psy_ui_ComboBox* sender, int sel);
static void playbar_onnumplaybeatsless(PlayBar*, psy_ui_Button* sender);
static void playbar_onnumplaybeatsmore(PlayBar*, psy_ui_Button* sender);
static void playbar_onplayclicked(PlayBar*, psy_ui_Component* sender);
static void playbar_startplay(PlayBar*);
static void playbar_onstopclicked(PlayBar*, psy_ui_Component* sender);
static void playbar_ontimer(PlayBar*, psy_ui_Component* sender, uintptr_t timerid);
static void playbar_onlanguagechanged(PlayBar*, psy_ui_Component* sender);

void playbar_init(PlayBar* self, psy_ui_Component* parent, Workspace* workspace)
{			
	self->workspace = workspace;
	self->player = &workspace->player;
	psy_ui_component_init(&self->component, parent);
	// ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	// loop
	psy_ui_button_init(&self->loop, &self->component);
	
	psy_signal_connect(&self->loop.signal_clicked, self, playbar_onloopclicked);
	// record
	psy_ui_button_init(&self->recordnotes, &self->component);
	
	psy_signal_connect(&self->recordnotes.signal_clicked, self, playbar_onrecordnotesclicked);
	psy_ui_button_init(&self->play, &self->component);
	
	psy_signal_connect(&self->play.signal_clicked, self, playbar_onplayclicked);
	// playmode
	psy_ui_combobox_init(&self->playmode, &self->component);	
	psy_ui_combobox_setcharnumber(&self->playmode, 6);	
	// play beat num
	psy_ui_edit_init(&self->loopbeatsedit, &self->component);
	psy_ui_edit_settext(&self->loopbeatsedit, "4.00");
	psy_ui_edit_setcharnumber(&self->loopbeatsedit, 6);
	psy_ui_button_init(&self->loopbeatsless, &self->component);
	psy_ui_button_seticon(&self->loopbeatsless, psy_ui_ICON_LESS);
	psy_signal_connect(&self->loopbeatsless.signal_clicked, self,
		playbar_onnumplaybeatsless);
	psy_ui_button_init(&self->loopbeatsmore, &self->component);
	psy_ui_button_seticon(&self->loopbeatsmore, psy_ui_ICON_MORE);
	psy_signal_connect(&self->loopbeatsmore.signal_clicked, self,
		playbar_onnumplaybeatsmore);
	// stop
	psy_ui_button_init(&self->stop, &self->component);	
	psy_signal_connect(&self->stop.signal_clicked, self, playbar_onstopclicked);
	psy_signal_connect(&self->component.signal_timer, self, playbar_ontimer);
	playbar_initalign(self);
	playbar_updatetext(self);
	psy_ui_combobox_setcursel(&self->playmode, 0);
	psy_signal_connect(&self->playmode.signal_selchanged, self,
		playbar_onplaymodeselchanged);
	psy_signal_connect(&self->workspace->signal_languagechanged, self,
		playbar_onlanguagechanged);
	psy_ui_component_starttimer(&self->component, TIMERID_PLAYBAR, 100);
}

void playbar_updatetext(PlayBar* self)
{
	psy_ui_button_settext(&self->loop, workspace_translate(self->workspace, "Loop"));
	psy_ui_button_settext(&self->recordnotes, workspace_translate(self->workspace, "Record Notes"));
	psy_ui_button_settext(&self->play, workspace_translate(self->workspace, "Play"));
	psy_ui_combobox_addtext(&self->playmode, workspace_translate(self->workspace, "Song"));
	psy_ui_combobox_addtext(&self->playmode, workspace_translate(self->workspace, "Sel"));
	psy_ui_combobox_addtext(&self->playmode, workspace_translate(self->workspace, "Beats"));
	psy_ui_button_settext(&self->stop, workspace_translate(self->workspace, "Stop"));
}

void playbar_initalign(PlayBar* self)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(0.5), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT, &margin));
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
	psy_dsp_beat_t playbeats;
	char text[40];
	
	playbeats = (psy_dsp_beat_t) atof(psy_ui_edit_text(&self->loopbeatsedit));
	if (playbeats > 1) {
		playbeats -= 1;
	}
	psy_audio_sequencer_setnumplaybeats(&self->player->sequencer, playbeats);
	psy_snprintf(text, 40, "%f", (double) playbeats);
	psy_ui_edit_settext(&self->loopbeatsedit, text);
}

void playbar_onnumplaybeatsmore(PlayBar* self, psy_ui_Button* sender)
{
	psy_dsp_beat_t playbeats;
	char text[40];
	
	playbeats = (psy_dsp_beat_t) atof(psy_ui_edit_text(&self->loopbeatsedit));	
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
	SequencePosition editposition;
	SequenceEntry* entry;
	psy_dsp_beat_t startposition;
	psy_dsp_beat_t numplaybeats;
	
	psy_ui_button_highlight(&self->play);
	sequence = self->player->sequencer.sequence;
	editposition = self->workspace->sequenceselection.editposition;
	entry = sequenceposition_entry(&editposition);
	if (entry) {
		psy_audio_exclusivelock_enter();		
		psy_audio_player_stop(self->player);
		startposition = entry->offset;
		if (psy_audio_sequencer_playmode(&self->player->sequencer)
			== psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS) {
			psy_audio_PatternEditPosition editposition;

			editposition = workspace_patterneditposition(self->workspace);			
			startposition += (psy_dsp_beat_t) editposition.offset;
			numplaybeats = (psy_dsp_beat_t) atof(psy_ui_edit_text(
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

void playbar_ontimer(PlayBar* self, psy_ui_Component* sender, uintptr_t timerid)
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

void playbar_onlanguagechanged(PlayBar* self, psy_ui_Component* sender)
{
	playbar_updatetext(self);
	psy_ui_component_align(playbar_base(self));
}
