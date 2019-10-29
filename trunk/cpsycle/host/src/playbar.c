// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "playbar.h"

void playbar_initalign(PlayBar*);
static void onrecordnotesclicked(PlayBar*, ui_component* sender);
static void onplayclicked(PlayBar*, ui_component* sender);
static void onstopclicked(PlayBar*, ui_component* sender);

void playbar_init(PlayBar* self, ui_component* parent, Workspace* workspace)
{			
	self->player = &workspace->player;
	ui_component_init(&self->component, parent);	
	ui_button_init(&self->recordnotes, &self->component);
	ui_button_settext(&self->recordnotes, "Record Notes");	
	signal_connect(&self->recordnotes.signal_clicked, self, onrecordnotesclicked);
	ui_button_init(&self->play, &self->component);
	ui_button_settext(&self->play, workspace_translate(workspace, "play"));
	signal_connect(&self->play.signal_clicked, self, onplayclicked);
	ui_button_init(&self->stop, &self->component);
	ui_button_settext(&self->stop, workspace_translate(workspace, "stop"));
	signal_connect(&self->stop.signal_clicked, self, onstopclicked);
	playbar_initalign(self);
}

void playbar_initalign(PlayBar* self)
{
	ui_margin margin = { 0, 5, 0, 0 };

	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT, &margin));
}

void onplayclicked(PlayBar* self, ui_component* sender)
{
	player_start(self->player);
}

void onstopclicked(PlayBar* self, ui_component* sender)
{
	player_stop(self->player);
}

void onrecordnotesclicked(PlayBar* self, ui_component* sender)
{	
	if (player_recordingnotes(self->player)) {
		player_stoprecordingnotes(self->player);
		ui_button_disablehighlight(&self->recordnotes);
	} else {
		player_startrecordingnotes(self->player);
		ui_button_highlight(&self->recordnotes);
	}
}
