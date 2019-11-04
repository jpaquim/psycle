// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songproperties.h"

static void songproperties_initalign(SongProperties*);
static void songproperties_read(SongProperties*);
static void songproperties_write(SongProperties*);
static void songproperties_onsongchanged(SongProperties*, Workspace*, int flag);
static void songproperties_onhide(SongProperties*, ui_component* sender);
static void songproperties_ontitlechanged(SongProperties*, ui_component* sender);
static void songproperties_oncreditschanged(SongProperties*, ui_component* sender);
static void songproperties_oncommentschanged(SongProperties*, ui_component* sender);

void songproperties_init(SongProperties* self, ui_component* parent,
	Workspace* workspace)
{		
	self->song = workspace->song;	
	ui_component_init(&self->component, parent);	
	ui_label_init(&self->label_title, &self->component);
	ui_label_settext(&self->label_title, "Song Title");
	ui_edit_init(&self->edit_title, &self->component, 0);
	signal_connect(&self->edit_title.signal_change, self,
		songproperties_ontitlechanged);
	ui_label_init(&self->label_credits, &self->component);
	ui_label_settext(&self->label_credits, "Credits");	
	ui_edit_init(&self->edit_credits, &self->component, 0);
	signal_connect(&self->edit_credits.signal_change, self,
		songproperties_oncreditschanged);
	ui_label_init(&self->label_comments, &self->component);
	ui_label_settext(&self->label_comments, "Comments");	
	ui_edit_init(&self->edit_comments, &self->component,
		WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL);
	ui_component_resize(&self->edit_comments.component, 0, 200);
	signal_connect(&self->edit_comments.signal_change, self,
		songproperties_oncommentschanged);
	songproperties_read(self);	
	signal_connect(&workspace->signal_songchanged, self,
		songproperties_onsongchanged);
	signal_connect(&self->component.signal_hide, self,
		songproperties_onhide);	
	songproperties_initalign(self);
}

void songproperties_initalign(SongProperties* self)
{
	ui_margin margin = { 0, 0, 0, 0 };

	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_TOP, &margin));
	ui_component_setalign(&self->edit_comments.component, UI_ALIGN_CLIENT);
}

void songproperties_read(SongProperties* self)
{
	char* text;

	ui_edit_settext(&self->edit_title, 
		properties_readstring(self->song->properties, "title", &text, ""));
	ui_edit_settext(&self->edit_credits, 
		properties_readstring(self->song->properties, "credits", &text, ""));
	ui_edit_settext(&self->edit_comments,
		properties_readstring(self->song->properties, "comments", &text, ""));
}

void songproperties_write(SongProperties* self)
{
	properties_write_string(self->song->properties, "title",
		ui_edit_text(&self->edit_title));
	properties_write_string(self->song->properties, "credits",
		ui_edit_text(&self->edit_credits));
	properties_write_string(self->song->properties, "comments",
		ui_edit_text(&self->edit_comments));
}

void songproperties_onsongchanged(SongProperties* self, Workspace* workspace,
	int flag)
{
	if (flag == WORKSPACE_LOADSONG) {
		songproperties_disableedit(self);
	} else {
		songproperties_enableedit(self);
	}
	self->song = workspace->song;
	songproperties_read(self);
}

void songproperties_enableedit(SongProperties* self)
{
	ui_edit_enableedit(&self->edit_title);
	ui_edit_enableedit(&self->edit_credits);	
	ui_edit_enableedit(&self->edit_comments);	
}

void songproperties_disableedit(SongProperties* self)
{
	ui_edit_preventedit(&self->edit_title);
	ui_edit_preventedit(&self->edit_credits);	
	ui_edit_preventedit(&self->edit_comments);	
}

void songproperties_onhide(SongProperties* self, ui_component* sender)
{
	songproperties_enableedit(self);
}

void songproperties_ontitlechanged(SongProperties* self, ui_component* sender)
{
	properties_write_string(self->song->properties, "title",
		ui_edit_text(&self->edit_title));		
}

void songproperties_oncreditschanged(SongProperties* self,
	ui_component* sender)
{
	properties_write_string(self->song->properties, "credits",
		ui_edit_text(&self->edit_credits));
}

void songproperties_oncommentschanged(SongProperties* self,
	ui_component* sender)
{
	properties_write_string(self->song->properties, "comments",
		ui_edit_text(&self->edit_comments));
}
