// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "playbar.h"
#include <exclusivelock.h>

#define TIMERID_PLAYBAR 400

static void playbar_initalign(PlayBar*);
static void onloopclicked(PlayBar*, psy_ui_Component* sender);
static void onrecordnotesclicked(PlayBar*, psy_ui_Component* sender);
static void onplayclicked(PlayBar*, psy_ui_Component* sender);
static void onplayselclicked(PlayBar*, psy_ui_Component* sender);
static void startplay(PlayBar*);
static void onstopclicked(PlayBar*, psy_ui_Component* sender);
static void ontimer(PlayBar*, psy_ui_Component* sender, int timerid);

void playbar_init(PlayBar* self, psy_ui_Component* parent, Workspace* workspace)
{			
	self->workspace = workspace;
	self->player = &workspace->player;
	ui_component_init(&self->component, parent);	
	ui_button_init(&self->loop, &self->component);
	ui_button_settext(&self->loop, "Loop");	
	psy_signal_connect(&self->loop.signal_clicked, self, onloopclicked);
	ui_button_init(&self->recordnotes, &self->component);
	ui_button_settext(&self->recordnotes, "Record Notes");	
	psy_signal_connect(&self->recordnotes.signal_clicked, self, onrecordnotesclicked);
	ui_button_init(&self->play, &self->component);
	ui_button_settext(&self->play, workspace_translate(workspace, "play"));
	psy_signal_connect(&self->play.signal_clicked, self, onplayclicked);
	ui_button_init(&self->playsel, &self->component);
	ui_button_settext(&self->playsel, workspace_translate(workspace, "sel"));
	psy_signal_connect(&self->playsel.signal_clicked, self, onplayselclicked);
	ui_button_init(&self->stop, &self->component);
	ui_button_settext(&self->stop, workspace_translate(workspace, "stop"));
	psy_signal_connect(&self->stop.signal_clicked, self, onstopclicked);
	psy_signal_connect(&self->component.signal_timer, self, ontimer);
	playbar_initalign(self);
	ui_component_starttimer(&self->component, TIMERID_PLAYBAR, 100);
}

void playbar_initalign(PlayBar* self)
{
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(0.5),
		ui_value_makepx(0), ui_value_makepx(0));
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	psy_list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT, &margin));
}

void onplayclicked(PlayBar* self, psy_ui_Component* sender)
{	
	startplay(self);
}

void onplayselclicked(PlayBar* self, psy_ui_Component* sender)
{
	if (sequencer_playmode(&self->player->sequencer) == SEQUENCERPLAYMODE_PLAYALL) {
		sequencer_setplaymode(&self->player->sequencer, SEQUENCERPLAYMODE_PLAYSEL);
		ui_button_highlight(&self->playsel);
	} else {
		sequencer_setplaymode(&self->player->sequencer, SEQUENCERPLAYMODE_PLAYALL);
		ui_button_disablehighlight(&self->playsel);
	}
	if (!player_playing(self->player)) {
		startplay(self);
	}
}

void startplay(PlayBar* self)
{
	psy_audio_Sequence* sequence;
	SequencePosition editposition;
	SequenceEntry* entry;
	
	ui_button_highlight(&self->play);
	sequence = self->player->sequencer.sequence;
	editposition = self->workspace->sequenceselection.editposition;
	entry = sequenceposition_entry(&editposition);
	if (entry) {
		lock_enter();
		player_stop(self->player);
		player_setposition(self->player, entry->offset);
		player_start(self->player);
		lock_leave();
	}
}

void onstopclicked(PlayBar* self, psy_ui_Component* sender)
{
	ui_button_disablehighlight(&self->play);
	ui_button_disablehighlight(&self->playsel);
	player_stop(self->player);	
}

void onloopclicked(PlayBar* self, psy_ui_Component* sender)
{
	if (sequencer_looping(&self->player->sequencer)) {
		sequencer_stoploop(&self->player->sequencer);
		ui_button_disablehighlight(&self->loop);
	} else {
		sequencer_loop(&self->player->sequencer);
		ui_button_highlight(&self->loop);
	}
}

void onrecordnotesclicked(PlayBar* self, psy_ui_Component* sender)
{	
	if (player_recordingnotes(self->player)) {
		player_stoprecordingnotes(self->player);
		ui_button_disablehighlight(&self->recordnotes);
	} else {
		player_startrecordingnotes(self->player);
		ui_button_highlight(&self->recordnotes);
	}
}

void ontimer(PlayBar* self, psy_ui_Component* sender, int timerid)
{
	if (player_playing(self->player)) {
		ui_button_highlight(&self->play);
		if (sequencer_playmode(&self->player->sequencer)
			== SEQUENCERPLAYMODE_PLAYSEL) {
			ui_button_highlight(&self->playsel);
		}
	} else {
		ui_button_disablehighlight(&self->play);
		ui_button_disablehighlight(&self->playsel);
	}
	if (sequencer_looping(&self->player->sequencer)) {
		ui_button_highlight(&self->loop);
	} else {
		ui_button_disablehighlight(&self->loop);
	}
}
