// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songproperties.h"

#include <songio.h>

#include <string.h>
#include <stdlib.h>

#include "../../detail/portable.h"

static void songpropertiesview_initalign(SongPropertiesView*);
static void songpropertiesview_read(SongPropertiesView*);
static void songpropertiesview_write(SongPropertiesView*);
static void songpropertiesview_onsongchanged(SongPropertiesView*, Workspace*,
	int flag, psy_audio_SongFile*);
static void songpropertiesview_onhide(SongPropertiesView*,
	psy_ui_Component* sender);
static void songpropertiesview_ontitlechanged(SongPropertiesView*,
	psy_ui_Component* sender);
static void songpropertiesview_oncreditschanged(SongPropertiesView*,
	psy_ui_Component* sender);
static void songpropertiesview_ontpbchanged(SongPropertiesView*,
	psy_ui_Component* sender);
static void songpropertiesview_oncommentschanged(SongPropertiesView*,
	psy_ui_Component* sender);

void songpropertiesview_init(SongPropertiesView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{		
	psy_ui_Margin margin;
	int charnum;

	charnum = 12;
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0), psy_ui_value_makeeh(1.0),
		psy_ui_value_makepx(0));
	self->song = workspace->song;	
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_init(&self->tabbar, tabbarparent);	
	// title
	psy_ui_component_init(&self->title, &self->component);
	psy_ui_component_enablealign(&self->title);
	psy_ui_label_init(&self->label_title, &self->title);
	psy_ui_label_settextalignment(&self->label_title, psy_ui_ALIGNMENT_RIGHT);
	psy_ui_label_settext(&self->label_title, "Song Title");
	psy_ui_label_setcharnumber(&self->label_title, charnum);
	psy_ui_component_setalign(&self->label_title.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin(&self->label_title.component, &margin);
	psy_ui_edit_init(&self->edit_title, &self->title);
	psy_ui_component_setalign(&self->edit_title.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setmargin(&self->edit_title.component, &margin);
	psy_signal_connect(&self->edit_title.signal_change, self,
		songpropertiesview_ontitlechanged);	
	// credits
	psy_ui_component_init(&self->credits, &self->component);
	psy_ui_component_enablealign(&self->credits);
	psy_ui_label_init(&self->label_credits, &self->credits);
	psy_ui_label_settext(&self->label_credits, "Credits");
	psy_ui_label_setcharnumber(&self->label_credits, charnum);
	psy_ui_component_setalign(&self->label_credits.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin(&self->label_credits.component, &margin);
	psy_ui_label_settextalignment(&self->label_credits, psy_ui_ALIGNMENT_RIGHT);
	psy_ui_edit_init(&self->edit_credits, &self->credits);
	psy_ui_component_setalign(&self->edit_credits.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setmargin(&self->edit_credits.component, &margin);
	psy_signal_connect(&self->edit_credits.signal_change, self,
		songpropertiesview_oncreditschanged);	
	// Speed
	psy_ui_component_init(&self->speed, &self->component);
	psy_ui_component_enablealign(&self->speed);
	psy_ui_label_init(&self->label_speed, &self->speed);
	psy_ui_label_settext(&self->label_speed, "Speed");
	psy_ui_label_setcharnumber(&self->label_speed, charnum);
	psy_ui_label_settextalignment(&self->label_speed, psy_ui_ALIGNMENT_RIGHT);
	psy_ui_label_init(&self->label_tpb, &self->speed);
	psy_ui_label_settext(&self->label_tpb, "Ticks Per Beat");
	psy_ui_edit_init(&self->edit_tpb, &self->speed);
	psy_ui_edit_setcharnumber(&self->edit_tpb, 4);
	psy_signal_connect(&self->edit_tpb.signal_change, self,
		songpropertiesview_ontpbchanged);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->speed, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));
	// Comments
	psy_ui_component_init(&self->comments, &self->component);
	psy_ui_component_enablealign(&self->comments);
	psy_ui_label_init(&self->label_comments, &self->comments);
	psy_ui_label_settext(&self->label_comments, "Extended Comments");
	psy_ui_label_settextalignment(&self->label_comments,
		psy_ui_ALIGNMENT_LEFT);
	//psy_ui_label_setcharnumber(&self->label_comments, charnum);
	psy_ui_component_setalign(&self->label_comments.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->label_comments.component, &margin);

	psy_ui_edit_multiline_init(&self->edit_comments, &self->component);
	psy_ui_component_setalign(&self->edit_comments.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setmargin(&self->edit_comments.component, &margin);
	psy_signal_connect(&self->edit_comments.signal_change, self,
		songpropertiesview_oncommentschanged);
	songpropertiesview_read(self);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		songpropertiesview_onsongchanged);
	psy_signal_connect(&self->component.signal_hide, self,
		songpropertiesview_onhide);
	songpropertiesview_initalign(self);	
}

void songpropertiesview_initalign(SongPropertiesView* self)
{	
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP,
		NULL));
	psy_ui_component_setalign(&self->edit_comments.component, psy_ui_ALIGN_CLIENT);
}

void songpropertiesview_read(SongPropertiesView* self)
{	
	char text[128];

	psy_ui_edit_settext(&self->edit_title, 
		self->song->properties.title);
	psy_ui_edit_settext(&self->edit_credits, 
		self->song->properties.credits);
	psy_ui_edit_settext(&self->edit_comments,
		self->song->properties.comments);
	psy_snprintf(text, 128, "%d", (int) self->song->properties.tpb);
	psy_ui_edit_settext(&self->edit_tpb, text);
}

void songpropertiesview_write(SongPropertiesView* self)
{
	int tpb;

	free(self->song->properties.title);
	self->song->properties.title = 
		strdup(psy_ui_edit_text(&self->edit_title));
	free(self->song->properties.credits);
	self->song->properties.credits = 
		strdup(psy_ui_edit_text(&self->edit_credits));
	free(self->song->properties.comments);
	self->song->properties.comments = 
		strdup(psy_ui_edit_text(&self->edit_comments));
	tpb = atoi(psy_ui_edit_text(&self->edit_tpb));
	self->song->properties.tpb = tpb;
}

void songpropertiesview_onsongchanged(SongPropertiesView* self,
	Workspace* workspace, int flag, psy_audio_SongFile* songfile)
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
	psy_ui_edit_enableedit(&self->edit_title);
	psy_ui_edit_enableedit(&self->edit_credits);
	psy_ui_edit_enableedit(&self->edit_tpb);
	psy_ui_edit_enableedit(&self->edit_comments);	
}

void songpropertiesview_disableedit(SongPropertiesView* self)
{
	psy_ui_edit_preventedit(&self->edit_title);
	psy_ui_edit_preventedit(&self->edit_credits);
	psy_ui_edit_preventedit(&self->edit_tpb);
	psy_ui_edit_preventedit(&self->edit_comments);	
}

void songpropertiesview_onhide(SongPropertiesView* self,
	psy_ui_Component* sender)
{
	songpropertiesview_enableedit(self);
}

void songpropertiesview_ontitlechanged(SongPropertiesView* self,
	psy_ui_Component* sender)
{
	free(self->song->properties.title);
	self->song->properties.title = strdup(psy_ui_edit_text(&self->edit_title));
}

void songpropertiesview_oncreditschanged(SongPropertiesView* self,
	psy_ui_Component* sender)
{
	free(self->song->properties.credits);
	self->song->properties.credits = strdup(psy_ui_edit_text(
		&self->edit_credits));
}

void songpropertiesview_ontpbchanged(SongPropertiesView* self,
	psy_ui_Component* sender)
{
	int tpb;
	
	tpb = atoi(psy_ui_edit_text(&self->edit_tpb));
	if (tpb == 0) {
		tpb = 24;
	}
	self->song->properties.tpb = tpb;
}

void songpropertiesview_oncommentschanged(SongPropertiesView* self,
	psy_ui_Component* sender)
{
	free(self->song->properties.comments);
	self->song->properties.comments = strdup(
		psy_ui_edit_text(&self->edit_comments));
}
