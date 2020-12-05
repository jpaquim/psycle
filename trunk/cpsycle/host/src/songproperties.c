// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songproperties.h"

#include <songio.h>
// std
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
// platform
#include "../../detail/portable.h"

// IntEdit
// prototypes
static void intedit_ondestroy(IntEdit*, psy_ui_Component* sender);
static void intedit_onlessclicked(IntEdit*, psy_ui_Component* sender);
static void intedit_onmoreclicked(IntEdit*, psy_ui_Component* sender);
static void intedit_oneditkeydown(IntEdit*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void intedit_oneditfocuslost(IntEdit*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);

// implementation
void intedit_init(IntEdit* self, psy_ui_Component* parent,
	const char* desc, int value, int minval, int maxval)
{
	psy_ui_component_init(intedit_base(self), parent);
	psy_ui_component_setalignexpand(intedit_base(self), psy_ui_HORIZONTALEXPAND);
	psy_ui_component_setdefaultalign(intedit_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->minval = minval;
	self->maxval = maxval;
	self->restore = value;
	psy_ui_label_init(&self->desc, intedit_base(self));
	psy_ui_label_settext(&self->desc, desc);
	psy_ui_edit_init(&self->edit, intedit_base(self));
	psy_ui_edit_setcharnumber(&self->edit, 4);	
	psy_ui_button_init_connect(&self->less, intedit_base(self),
		self, intedit_onlessclicked);
	psy_ui_button_seticon(&self->less, psy_ui_ICON_LESS);
	psy_ui_button_init_connect(&self->more, intedit_base(self),
		self, intedit_onmoreclicked);
	psy_ui_button_seticon(&self->more, psy_ui_ICON_MORE);
	psy_signal_init(&self->signal_changed);
	intedit_setvalue(self, value);
	psy_signal_connect(&self->edit.component.signal_keydown, self,
		intedit_oneditkeydown);	
	psy_signal_connect(&self->edit.component.signal_focuslost, self,
		intedit_oneditfocuslost);
	psy_signal_connect(&self->component.signal_destroy, self,
		intedit_ondestroy);
}

void intedit_init_connect(IntEdit* self, psy_ui_Component* parent,
	const char* desc, int value, int minval, int maxval,
	void* context, void* fp)
{
	intedit_init(self, parent, desc, value, minval, maxval);
	psy_signal_connect(&self->signal_changed, context, fp);
}

void intedit_ondestroy(IntEdit* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_changed);
}

int intedit_value(IntEdit* self)
{
	return atoi(psy_ui_edit_text(&self->edit));
}

void intedit_setvalue(IntEdit* self, int value)
{
	char text[128];

	value = psy_min(psy_max(value, self->minval), self->maxval);		
	psy_snprintf(text, 128, "%d", value);
	psy_ui_edit_settext(&self->edit, text);
	psy_signal_emit(&self->signal_changed, self, 0);
	self->restore = value;
}

void intedit_enableedit(IntEdit* self)
{
	psy_ui_edit_enableedit(&self->edit);
}

void intedit_preventedit(IntEdit* self)
{
	psy_ui_edit_preventedit(&self->edit);
}

void intedit_onlessclicked(IntEdit* self, psy_ui_Component* sender)
{
	intedit_setvalue(self, intedit_value(self) - 1);
}

void intedit_onmoreclicked(IntEdit* self, psy_ui_Component* sender)
{
	intedit_setvalue(self, intedit_value(self) + 1);
}

void intedit_oneditkeydown(IntEdit* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{
	if (isalpha(ev->keycode) || ev->keycode == psy_ui_KEY_ESCAPE) {
		intedit_setvalue(self, self->restore);
		psy_ui_keyevent_preventdefault(ev);
		return;
	}
	if (ev->keycode == psy_ui_KEY_RETURN) {
		int value;

		psy_ui_component_setfocus(&self->component);
		psy_ui_keyevent_preventdefault(ev);
		value = intedit_value(self);
		value = psy_min(psy_max(value, self->minval), self->maxval);
		intedit_setvalue(self, value);
		psy_signal_emit(&self->signal_changed, self, 0);
	}
}

void intedit_oneditfocuslost(IntEdit* self , psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{
	int value;

	psy_ui_keyevent_preventdefault(ev);
	value = intedit_value(self);
	intedit_setvalue(self, value);
	psy_signal_emit(&self->signal_changed, self, 0);
}

static void songpropertiesview_initalign(SongPropertiesView*);
static void songpropertiesview_read(SongPropertiesView*);
static void songpropertiesview_onsongchanged(SongPropertiesView*, Workspace*,
	int flag, psy_audio_SongFile*);
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
static int songpropertiesview_realtpb(SongPropertiesView*);
static void songpropertiesview_updaterealspeed(SongPropertiesView*);

void songpropertiesview_init(SongPropertiesView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{		
	psy_ui_Margin margin;
	int charnum;

	charnum = 12;
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0), psy_ui_value_makeeh(1.0),
		psy_ui_value_makeew(3.0));
	self->song = workspace->song;
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_init(&self->tabbar, tabbarparent);	
	// title
	psy_ui_component_init(&self->title, &self->component);
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
	psy_signal_connect(&self->edit_title.component.signal_keydown, self,
		songpropertiesview_onfilterkeys);
	// credits
	psy_ui_component_init(&self->credits, &self->component);	
	psy_ui_label_init_text(&self->label_credits, &self->credits,
		"Credits");
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
		"Speed");
	psy_ui_component_init(&self->speedbar, &self->speed);
	psy_ui_component_setalign(&self->speedbar, psy_ui_ALIGN_LEFT);
	psy_ui_label_setcharnumber(&self->label_speed, charnum);
	psy_ui_component_setdefaultalign(&self->speedbar, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make(psy_ui_value_makepx(0),
			psy_ui_value_makeew(1.0), psy_ui_value_makepx(0),
			psy_ui_value_makepx(0)));		
	// Tempo
	intedit_init_connect(&self->tempo, &self->speedbar, "Tempo", 32, 32, 999,
		self, songpropertiesview_ontempoeditchange);
	intedit_init_connect(&self->lpb, &self->speedbar, "Lines Per Beat",
		1, 1, 32, self, songpropertiesview_onlpbeditchange);
	intedit_init_connect(&self->tpb, &self->speedbar, "Ticks Per Beat",
		1, 1, 99, self, songpropertiesview_ontpbeditchange);
	intedit_init_connect(&self->etpb, &self->speedbar, "Extra Ticks Per Beat",
		0, 0, 99, self, songpropertiesview_onetpbeditchange);
	// Real Speed
	psy_ui_label_init_text(&self->realtempo_desc, &self->speedbar,
		"Real tempo");
	psy_ui_label_init(&self->realtempo, &self->speedbar);
	psy_ui_label_setcharnumber(&self->realtempo, 8);
	psy_ui_label_init_text(&self->realticksperbeat_desc, &self->speedbar,
		"Real ticks per beat");	
	psy_ui_label_init(&self->realticksperbeat, &self->speedbar);
	psy_ui_label_setcharnumber(&self->realticksperbeat, 8);		
	// Comments
	psy_ui_component_init(&self->comments, &self->component);
	psy_ui_label_init_text(&self->label_comments, &self->comments,
		"Extended Comments");
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
	psy_audio_player_setbpm(&self->workspace->player, intedit_value(sender));
	songpropertiesview_updaterealspeed(self);
}

void songpropertiesview_onlpbeditchange(SongPropertiesView* self,
	IntEdit* sender)
{
	psy_audio_player_setlpb(&self->workspace->player, (uintptr_t)
		intedit_value(sender));
	songpropertiesview_updaterealspeed(self);
}

void songpropertiesview_ontpbeditchange(SongPropertiesView* self,
	IntEdit* sender)
{
	psy_audio_player_setticksperbeat(&self->workspace->player, (uintptr_t)
		intedit_value(sender));
	songpropertiesview_updaterealspeed(self);
}

void songpropertiesview_onetpbeditchange(SongPropertiesView* self,
	IntEdit* sender)
{
	psy_audio_player_setextraticksperbeat(&self->workspace->player, (uintptr_t)
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
int songpropertiesview_realtpb(SongPropertiesView* self)
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
