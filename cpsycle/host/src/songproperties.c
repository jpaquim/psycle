// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songproperties.h"

static void ReadProperties(SongProperties*, Song*);
static void WriteProperties(SongProperties*, Song*);
static void OnSongChanged(SongProperties* self, Workspace* sender);
static void OnSize(SongProperties*, ui_component* sender, int width, int height);
static void songproperties_initalign(SongProperties*);

void InitSongProperties(SongProperties* self, ui_component* parent, Workspace* workspace)
{		
	self->song = workspace->song;
	self->song->properties = properties_create();
	properties_append_string(self->song->properties, "title", "a title");
	properties_append_string(self->song->properties, "credits", "the credits");
	properties_append_string(self->song->properties, "comments", "the comments");
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);		
	ui_label_init(&self->label_title, &self->component);
	ui_label_settext(&self->label_title, "Song Title");		
	ui_edit_init(&self->edit_title, &self->component, ES_READONLY);	
	ui_label_init(&self->label_credits, &self->component);
	ui_label_settext(&self->label_credits, "Credits");	
	ui_edit_init(&self->edit_credits, &self->component, ES_READONLY);
	ui_label_init(&self->label_comments, &self->component);
	ui_label_settext(&self->label_comments, "Comments");	
	ui_edit_init(&self->edit_comments, &self->component,
		WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL);
	ui_component_resize(&self->edit_comments.component, 0, 200);
	ReadProperties(self, self->song);	
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
	songproperties_initalign(self);
}

void songproperties_initalign(SongProperties* self)
{
	ui_margin margin = { 0, 0, 0, 0 };

	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	ui_components_setalign(ui_component_children(&self->component, 0),
		UI_ALIGN_TOP, &margin);
	ui_component_setalign(&self->edit_comments.component, UI_ALIGN_CLIENT);
}

void ReadProperties(SongProperties* self, Song* song)
{
	ui_edit_settext(&self->edit_title, properties_read(song->properties, "title")->item.value.s);
	ui_edit_settext(&self->edit_credits, properties_read(song->properties, "credits")->item.value.s);	
	ui_edit_settext(&self->edit_comments, properties_read(song->properties, "comments")->item.value.s);
}

void WriteProperties(SongProperties* self, Song* song)
{
	properties_write_string(song->properties, "title", ui_edit_text(&self->edit_title));
}

void OnSongChanged(SongProperties* self, Workspace* workspace)
{
	self->song = workspace->song;
	ReadProperties(self, self->song);
}
