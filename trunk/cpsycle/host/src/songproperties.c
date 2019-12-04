// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songproperties.h"

static void songpropertiesview_initalign(SongPropertiesView*);
static void songpropertiesview_read(SongPropertiesView*);
static void songpropertiesview_write(SongPropertiesView*);
static void songpropertiesview_onsongchanged(SongPropertiesView*, Workspace*, int flag);
static void songpropertiesview_onhide(SongPropertiesView*, ui_component* sender);
static void songpropertiesview_ontitlechanged(SongPropertiesView*, ui_component* sender);
static void songpropertiesview_oncreditschanged(SongPropertiesView*, ui_component* sender);
static void songpropertiesview_oncommentschanged(SongPropertiesView*, ui_component* sender);

void songpropertiesview_init(SongPropertiesView* self, ui_component* parent,
	Workspace* workspace)
{		
	self->song = workspace->song;	
	ui_component_init(&self->component, parent);	
	ui_label_init(&self->label_title, &self->component);
	ui_label_settext(&self->label_title, "Song Title");
	ui_edit_init(&self->edit_title, &self->component, 0);
	signal_connect(&self->edit_title.signal_change, self,
		songpropertiesview_ontitlechanged);
	ui_label_init(&self->label_credits, &self->component);
	ui_label_settext(&self->label_credits, "Credits");	
	ui_edit_init(&self->edit_credits, &self->component, 0);
	signal_connect(&self->edit_credits.signal_change, self,
		songpropertiesview_oncreditschanged);
	ui_label_init(&self->label_comments, &self->component);
	ui_label_settext(&self->label_comments, "Comments");	
	ui_edit_init(&self->edit_comments, &self->component,
		WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL);
	ui_component_resize(&self->edit_comments.component, 0, 200);
	signal_connect(&self->edit_comments.signal_change, self,
		songpropertiesview_oncommentschanged);
	songpropertiesview_read(self);	
	signal_connect(&workspace->signal_songchanged, self,
		songpropertiesview_onsongchanged);
	signal_connect(&self->component.signal_hide, self,
		songpropertiesview_onhide);	
	songpropertiesview_initalign(self);
}

void songpropertiesview_initalign(SongPropertiesView* self)
{
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makepx(0),
		ui_value_makepx(0), ui_value_makepx(0));
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_TOP, &margin));
	ui_component_setalign(&self->edit_comments.component, UI_ALIGN_CLIENT);
}

void songpropertiesview_read(SongPropertiesView* self)
{	
	ui_edit_settext(&self->edit_title, 
		self->song->properties.title);
	ui_edit_settext(&self->edit_credits, 
		self->song->properties.credits);
	ui_edit_settext(&self->edit_comments,
		self->song->properties.comments);
}

void songpropertiesview_write(SongPropertiesView* self)
{
	free(self->song->properties.title);
	self->song->properties.title = 
		strdup(ui_edit_text(&self->edit_title));
	free(self->song->properties.credits);
	self->song->properties.credits = 
		strdup(ui_edit_text(&self->edit_credits));
	free(self->song->properties.comments);
	self->song->properties.comments = 
		strdup(ui_edit_text(&self->edit_comments));
}

void songpropertiesview_onsongchanged(SongPropertiesView* self,
	Workspace* workspace, int flag)
{
	if (flag == WORKSPACE_LOADSONG) {
		songpropertiesview_disableedit(self);
	} else {
		songpropertiesview_enableedit(self);
	}
	self->song = workspace->song;
	songpropertiesview_read(self);
}

void songpropertiesview_enableedit(SongPropertiesView* self)
{
	ui_edit_enableedit(&self->edit_title);
	ui_edit_enableedit(&self->edit_credits);	
	ui_edit_enableedit(&self->edit_comments);	
}

void songpropertiesview_disableedit(SongPropertiesView* self)
{
	ui_edit_preventedit(&self->edit_title);
	ui_edit_preventedit(&self->edit_credits);	
	ui_edit_preventedit(&self->edit_comments);	
}

void songpropertiesview_onhide(SongPropertiesView* self, ui_component* sender)
{
	songpropertiesview_enableedit(self);
}

void songpropertiesview_ontitlechanged(SongPropertiesView* self, ui_component* sender)
{
	free(self->song->properties.title);
	self->song->properties.title = strdup(ui_edit_text(&self->edit_title));
}

void songpropertiesview_oncreditschanged(SongPropertiesView* self,
	ui_component* sender)
{
	free(self->song->properties.credits);
	self->song->properties.credits = strdup(ui_edit_text(&self->edit_credits));
}

void songpropertiesview_oncommentschanged(SongPropertiesView* self,
	ui_component* sender)
{
	free(self->song->properties.comments);
	self->song->properties.comments = strdup(
		ui_edit_text(&self->edit_comments));
}
