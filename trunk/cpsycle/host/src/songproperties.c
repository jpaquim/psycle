// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songproperties.h"
// host
#include "styles.h"
// audio
#include <songio.h>
// std
#include <ctype.h>
// platform
#include "../../detail/portable.h"

// prototypes
static void songpropertiesview_initalign(SongPropertiesView*);
static void songpropertiesview_read(SongPropertiesView*);
static void songpropertiesview_onsongchanged(SongPropertiesView*, Workspace*,
	int flag, psy_audio_Song*);
static void songpropertiesview_onhide(SongPropertiesView*,
	psy_ui_Component* sender);
static void songpropertiesview_ontitlechanged(SongPropertiesView*,
	psy_ui_Component* sender);
static void songpropertiesview_onfilterkeys(SongPropertiesView*,
	psy_ui_Component* sender, psy_ui_KeyEvent*);
static void songpropertiesview_oncreditschanged(SongPropertiesView*,
	psy_ui_Component* sender);
static void songpropertiesview_ontempoeditchange(SongPropertiesView*,
	IntEdit* sender);
static void songpropertiesview_onlpbeditchange(SongPropertiesView*,
	IntEdit* sender);
static void songpropertiesview_ontpbeditchange(SongPropertiesView*,
	IntEdit* sender);
static void songpropertiesview_onetpbeditchange(SongPropertiesView*,
	IntEdit* sender);
static void songpropertiesview_oncommentschanged(SongPropertiesView*,
	psy_ui_Component* sender);
static void songpropertiesview_onkeydown(SongPropertiesView*,
	psy_ui_Component* sender, psy_ui_KeyEvent*);
static void songpropertiesview_onkeyup(SongPropertiesView*,
	psy_ui_Component* sender, psy_ui_KeyEvent*);
static bool songpropertiesview_haseditfocus(SongPropertiesView*);
static songpropertiesview_realbpm(SongPropertiesView*);
static uintptr_t songpropertiesview_realtpb(SongPropertiesView*);
static void songpropertiesview_updaterealspeed(SongPropertiesView*);
// implementation
void songpropertiesview_init(SongPropertiesView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{		
	psy_ui_Margin margin;
	double charnum;

	charnum = 12.0;
	psy_ui_margin_init_all_em(&margin, 0.0, 2.0, 1.0, 3.0);
	self->song = workspace->song;
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_init(&self->tabbar, tabbarparent);	
	// title
	psy_ui_component_init(&self->title, &self->component);
	psy_ui_component_setstyletypes(&self->component,
		STYLE_SONGPROPERTIES,
		STYLE_SONGPROPERTIES,
		STYLE_SONGPROPERTIES);
	psy_ui_label_init(&self->label_title, &self->title);
	psy_ui_label_settextalignment(&self->label_title, psy_ui_ALIGNMENT_RIGHT);
	psy_ui_label_settext(&self->label_title, "songproperties.title");
	psy_ui_label_setcharnumber(&self->label_title, charnum);
	psy_ui_component_setalign(&self->label_title.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin(&self->label_title.component, &margin);
	psy_ui_edit_init(&self->edit_title, &self->title);
	psy_ui_component_setalign(&self->edit_title.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setmargin(&self->edit_title.component, &margin);
	psy_signal_connect(&self->edit_title.signal_change, self,
		songpropertiesview_ontitlechanged);
	psy_signal_connect(&self->edit_title.component.signal_keydown, self,
		songpropertiesview_onfilterkeys);
	// credits
	psy_ui_component_init(&self->credits, &self->component);	
	psy_ui_label_init_text(&self->label_credits, &self->credits,
		"songproperties.credits");
	psy_ui_label_setcharnumber(&self->label_credits, charnum);
	psy_ui_component_setalign(&self->label_credits.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin(&self->label_credits.component, &margin);
	psy_ui_label_settextalignment(&self->label_credits, psy_ui_ALIGNMENT_RIGHT);
	psy_ui_edit_init(&self->edit_credits, &self->credits);
	psy_ui_component_setalign(&self->edit_credits.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setmargin(&self->edit_credits.component, &margin);
	psy_signal_connect(&self->edit_credits.signal_change, self,
		songpropertiesview_oncreditschanged);
	psy_signal_connect(&self->edit_credits.component.signal_keydown, self,
		songpropertiesview_onfilterkeys);
	// Speed
	psy_ui_component_init(&self->speed, &self->component);	
	psy_ui_component_setdefaultalign(&self->speed, psy_ui_ALIGN_LEFT,
			margin);
	psy_ui_label_init_text(&self->label_speed, &self->speed,
		"songproperties.speed");
	psy_ui_component_init(&self->speedbar, &self->speed);
	psy_ui_component_setalign(&self->speedbar, psy_ui_ALIGN_LEFT);
	psy_ui_label_setcharnumber(&self->label_speed, charnum);
	psy_ui_component_setdefaultalign(&self->speedbar, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make(psy_ui_value_makepx(0),
			psy_ui_value_makeew(1.0), psy_ui_value_makepx(0),
			psy_ui_value_makepx(0)));		
	// Tempo
	intedit_init_connect(&self->tempo, &self->speedbar, "songproperties.tempo", 32, 32, 999,
		self, songpropertiesview_ontempoeditchange);
	intedit_init_connect(&self->lpb, &self->speedbar, "songproperties.lpb",
		1, 1, 32, self, songpropertiesview_onlpbeditchange);
	intedit_init_connect(&self->tpb, &self->speedbar, "songproperties.tpb",
		1, 1, 99, self, songpropertiesview_ontpbeditchange);
	intedit_init_connect(&self->etpb, &self->speedbar, "songproperties.etpb",
		0, 0, 99, self, songpropertiesview_onetpbeditchange);
	// Real Speed
	psy_ui_label_init_text(&self->realtempo_desc, &self->speedbar,
		"songproperties.realtempo");
	psy_ui_label_init(&self->realtempo, &self->speedbar);
	psy_ui_label_setcharnumber(&self->realtempo, 8);
	psy_ui_label_init_text(&self->realticksperbeat_desc, &self->speedbar,
		"songproperties.realtpb");	
	psy_ui_label_init(&self->realticksperbeat, &self->speedbar);
	psy_ui_label_setcharnumber(&self->realticksperbeat, 8);		
	// Comments
	psy_ui_component_init(&self->comments, &self->component);
	psy_ui_component_setstyletypes(&self->comments,
		STYLE_SONGPROPERTIES_COMMENTS,
		STYLE_SONGPROPERTIES_COMMENTS,
		STYLE_SONGPROPERTIES_COMMENTS);
	psy_ui_label_init_text(&self->label_comments, &self->comments,
		"songproperties.extcomments");
	psy_ui_label_settextalignment(&self->label_comments,
		psy_ui_ALIGNMENT_LEFT);
	//psy_ui_label_setcharnumber(&self->label_comments, charnum);
	psy_ui_component_setalign(&self->label_comments.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->label_comments.component, &margin);
	psy_ui_edit_multiline_init(&self->edit_comments, &self->component);
	psy_ui_component_setstyletypes(&self->edit_comments.component,
		STYLE_SONGPROPERTIES_COMMENTS,
		STYLE_SONGPROPERTIES_COMMENTS,
		STYLE_SONGPROPERTIES_COMMENTS);
	psy_ui_component_setalign(&self->edit_comments.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setmargin(&self->edit_comments.component, &margin);
	psy_signal_connect(&self->edit_comments.signal_change, self,
		songpropertiesview_oncommentschanged);
	psy_signal_connect(&self->edit_comments.component.signal_keydown, self,
		songpropertiesview_onfilterkeys);
	songpropertiesview_read(self);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		songpropertiesview_onsongchanged);
	psy_signal_connect(&self->component.signal_hide, self,
		songpropertiesview_onhide);
	psy_signal_connect(&self->component.signal_keydown, self,
		songpropertiesview_onkeydown);
	psy_signal_connect(&self->component.signal_keyup, self,
		songpropertiesview_onkeyup);
	songpropertiesview_initalign(self);	
}

void songpropertiesview_initalign(SongPropertiesView* self)
{
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP,
		NULL));
	psy_ui_component_setalign(&self->edit_comments.component, psy_ui_ALIGN_CLIENT);
}

void songpropertiesview_read(SongPropertiesView* self)
{	
	if (self->song) {
		psy_ui_edit_settext(&self->edit_title,
			self->song->properties.title);
		psy_ui_edit_settext(&self->edit_credits,
			self->song->properties.credits);
		psy_ui_edit_settext(&self->edit_comments,
			self->song->properties.comments);
		intedit_setvalue(&self->tempo, (int)self->song->properties.bpm);
		intedit_setvalue(&self->lpb, (int)self->song->properties.lpb);
		intedit_setvalue(&self->tpb, (int)self->song->properties.tpb);
		intedit_setvalue(&self->etpb, (int)
			self->song->properties.extraticksperbeat);
		songpropertiesview_updaterealspeed(self);
	}
}

void songpropertiesview_onsongchanged(SongPropertiesView* self,
	Workspace* workspace, int flag, psy_audio_Song* song)
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
	psy_ui_edit_enableedit(&self->edit_comments);
	intedit_enableedit(&self->tpb);
	intedit_enableedit(&self->etpb);	
}

void songpropertiesview_disableedit(SongPropertiesView* self)
{
	psy_ui_edit_preventedit(&self->edit_title);
	psy_ui_edit_preventedit(&self->edit_credits);
	psy_ui_edit_preventedit(&self->edit_comments);
	intedit_preventedit(&self->tpb);
	intedit_preventedit(&self->etpb);	
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

void songpropertiesview_onfilterkeys(SongPropertiesView* self,
	psy_ui_Component* sender, psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_RETURN && sender != &self->edit_comments.component) {
		psy_ui_component_setfocus(&self->component);		
		psy_ui_keyevent_preventdefault(ev);
	} else
	if (ev->keycode == psy_ui_KEY_ESCAPE) {
		psy_ui_component_setfocus(&self->component);
		psy_ui_keyevent_preventdefault(ev);
	}	
}

void songpropertiesview_oncreditschanged(SongPropertiesView* self,
	psy_ui_Component* sender)
{
	free(self->song->properties.credits);
	self->song->properties.credits = strdup(psy_ui_edit_text(
		&self->edit_credits));
}

void songpropertiesview_ontempoeditchange(SongPropertiesView* self,
	IntEdit* sender)
{
	psy_audio_player_setbpm(workspace_player(self->workspace), intedit_value(sender));
	songpropertiesview_updaterealspeed(self);
}

void songpropertiesview_onlpbeditchange(SongPropertiesView* self,
	IntEdit* sender)
{
	psy_audio_player_setlpb(workspace_player(self->workspace), (uintptr_t)
		intedit_value(sender));
	songpropertiesview_updaterealspeed(self);
}

void songpropertiesview_ontpbeditchange(SongPropertiesView* self,
	IntEdit* sender)
{
	psy_audio_player_setticksperbeat(workspace_player(self->workspace), (uintptr_t)
		intedit_value(sender));
	songpropertiesview_updaterealspeed(self);
}

void songpropertiesview_onetpbeditchange(SongPropertiesView* self,
	IntEdit* sender)
{
	psy_audio_player_setextraticksperbeat(workspace_player(self->workspace), (uintptr_t)
		intedit_value(sender));
	songpropertiesview_updaterealspeed(self);
}

void songpropertiesview_oncommentschanged(SongPropertiesView* self,
	psy_ui_Component* sender)
{
	psy_strreset(&self->song->properties.comments,
		psy_ui_edit_text(&self->edit_comments));
}

void songpropertiesview_onkeydown(SongPropertiesView* self,
	psy_ui_Component* sender, psy_ui_KeyEvent* ev)
{
	if (songpropertiesview_haseditfocus(self)) {		
		psy_ui_keyevent_stoppropagation(ev);
	}
}

bool songpropertiesview_haseditfocus(SongPropertiesView* self)
{
	return (psy_ui_component_hasfocus(&self->edit_comments.component) ||
		psy_ui_component_hasfocus(&self->edit_credits.component) ||
		psy_ui_component_hasfocus(&self->edit_title.component));
		//psy_ui_component_hasfocus(&self->edit_tpb.component));
}

void songpropertiesview_onkeyup(SongPropertiesView* self,
	psy_ui_Component* sender, psy_ui_KeyEvent* ev)
{
	psy_ui_keyevent_stoppropagation(ev);
}

int songpropertiesview_realbpm(SongPropertiesView* self)
{
	return (int)((self->song->properties.bpm *
		self->song->properties.tpb) /
		(float)(self->song->properties.extraticksperbeat *
			self->song->properties.lpb +
			self->song->properties.tpb));
}
uintptr_t songpropertiesview_realtpb(SongPropertiesView* self)
{	
	return self->song->properties.tpb +
		(self->song->properties.extraticksperbeat *
			self->song->properties.lpb);
}

void songpropertiesview_updaterealspeed(SongPropertiesView* self)
{
	char text[128];

	psy_snprintf(text, 128, "%d", (int)
		songpropertiesview_realbpm(self));
	psy_ui_label_settext(&self->realtempo, text);
	psy_snprintf(text, 128, "%d", (int)
		songpropertiesview_realtpb(self));
	psy_ui_label_settext(&self->realticksperbeat, text);
}
