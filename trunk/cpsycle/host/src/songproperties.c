// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "songproperties.h"

static int Align(SongProperties* self, ui_component* child);
static void ReadProperties(SongProperties* self, Song* song);
static void WriteProperties(SongProperties* self, Song* song);
static void OnOkClicked(SongProperties* self);

static int fieldheight = 20;
static int linestep = 30;
static int cpy = 10;


void InitSongProperties(SongProperties* self, ui_component* parent, Song* song)
{			
	self->song = song;
	properties_append_string(song->properties, "title", "a title");
	properties_append_string(song->properties, "credits", "the credits");
	properties_append_string(song->properties, "comments", "the comments");
	ui_component_init(self, &self->component, parent);	
	ui_component_move(&self->component, 200, 150);
	ui_component_resize(&self->component, 400, 400);

	ui_label_init(&self->label_title, &self->component);
	ui_label_settext(&self->label_title, "Song Title");	
	ui_edit_init(&self->edit_title, &self->component, 0);	
	ui_label_init(&self->label_credits, &self->component);
	ui_label_settext(&self->label_credits, "Credits");	
	ui_edit_init(&self->edit_credits, &self->component, 0);
	ui_label_init(&self->label_comments, &self->component);
	ui_label_settext(&self->label_comments, "Comments");	
	ui_edit_init(&self->edit_comments, &self->component,
		WS_VSCROLL | ES_MULTILINE |ES_AUTOHSCROLL | ES_AUTOVSCROLL);
	self->edit_comments.component.align = 1;	
	ui_button_init(&self->button_ok, &self->component);
	ui_button_connect(&self->button_ok, self);
	self->button_ok.events.clicked = OnOkClicked;
	ui_button_settext(&self->button_ok, "OK");
	ReadProperties(self, song);

	self->component.events.childenum = Align;
	ui_component_enum_children(&self->component);
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

int Align(SongProperties* self, ui_component* child)
{	
	ui_component_move(child, 10, cpy);
	if (child->align == 1) {
		ui_component_resize(child, 200, 100);
		cpy += 110;
	} else {
		ui_component_resize(child, 200, fieldheight);
		cpy += linestep;
	}	
	return 1;
}

void OnOkClicked(SongProperties* self)
{
	WriteProperties(self, self->song);
}