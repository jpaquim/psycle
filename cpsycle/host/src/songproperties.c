/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "songproperties.h"
/* host */
#include "styles.h"
/* audio */
#include <songio.h>
/* std */
#include <ctype.h>
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void songpropertiesview_read(SongPropertiesView*);
static void songpropertiesview_onsongchanged(SongPropertiesView*,
	Workspace* sender);
static void songpropertiesview_onhide(SongPropertiesView*,
	psy_ui_Component* sender);
static void songpropertiesview_oneditaccept(SongPropertiesView*,
	psy_ui_TextArea* sender);
static void songpropertiesview_oneditreject(SongPropertiesView*,
	psy_ui_TextArea* sender);
static void songpropertiesview_onfilterkeys(SongPropertiesView*,
	psy_ui_Component* sender, psy_ui_KeyboardEvent*);
static void songpropertiesview_ontempoeditchange(SongPropertiesView*,
	IntEdit* sender);
static void songpropertiesview_onlpbeditchange(SongPropertiesView*,
	IntEdit* sender);
static void songpropertiesview_ontpbeditchange(SongPropertiesView*,
	IntEdit* sender);
static void songpropertiesview_onetpbeditchange(SongPropertiesView*,
	IntEdit* sender);
static void songpropertiesview_onsamplerindexchange(SongPropertiesView*,
	IntEdit* sender);
static void songpropertiesview_oncommentschanged(SongPropertiesView*,
	psy_ui_Component* sender);
static void songpropertiesview_on_key_down(SongPropertiesView*,
	psy_ui_Component* sender, psy_ui_KeyboardEvent*);
static void songpropertiesview_onkeyup(SongPropertiesView*,
	psy_ui_Component* sender, psy_ui_KeyboardEvent*);
static bool songpropertiesview_haseditfocus(SongPropertiesView*);
static int songpropertiesview_realbpm(SongPropertiesView*);
static uintptr_t songpropertiesview_realtpb(SongPropertiesView*);
static void songpropertiesview_updaterealspeed(SongPropertiesView*);

/* implementation */
void songpropertiesview_init(SongPropertiesView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{		
	psy_ui_Margin margin;
	double charnum;

	charnum = 12.0;
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 1.0, 3.0);
	self->song = workspace->song;
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_id(songpropertiesview_base(self), VIEW_ID_SONGPROPERTIES);
	psy_ui_component_set_title(songpropertiesview_base(self), "main.properties");
	psy_ui_component_set_style_type(&self->component, STYLE_SONGPROPERTIES);
	psy_ui_component_init(&self->viewtabbar, tabbarparent, NULL);
	psy_ui_component_init(&self->top, &self->component, NULL);
	psy_ui_component_set_align(&self->top, psy_ui_ALIGN_TOP);
	psy_ui_component_set_default_align(&self->top, psy_ui_ALIGN_TOP,
		psy_ui_margin_zero());
	/* title */
	psy_ui_component_init(&self->title, &self->top, NULL);	
	psy_ui_label_init(&self->label_title, &self->title);
	psy_ui_label_set_textalignment(&self->label_title, psy_ui_ALIGNMENT_RIGHT);
	psy_ui_label_set_text(&self->label_title, "songproperties.title");
	psy_ui_label_set_char_number(&self->label_title, charnum);
	psy_ui_component_set_align(&self->label_title.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_set_margin(&self->label_title.component, margin);
	psy_ui_textarea_init_single_line(&self->edit_title, &self->title);	
	psy_ui_textarea_enable_input_field(&self->edit_title);
	psy_ui_component_set_align(&self->edit_title.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_margin(&self->edit_title.component, margin);
	psy_signal_connect(&self->edit_title.signal_accept, self,
		songpropertiesview_oneditaccept);
	psy_signal_connect(&self->edit_title.signal_reject, self,
		songpropertiesview_oneditreject);
	psy_signal_connect(&self->edit_title.component.signal_keydown, self,
		songpropertiesview_onfilterkeys);
	/* credits */
	psy_ui_component_init(&self->credits, &self->top, NULL);
	psy_ui_label_init_text(&self->label_credits, &self->credits,
		"songproperties.credits");
	psy_ui_label_set_char_number(&self->label_credits, charnum);
	psy_ui_component_set_align(&self->label_credits.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_set_margin(&self->label_credits.component, margin);
	psy_ui_label_set_textalignment(&self->label_credits, psy_ui_ALIGNMENT_RIGHT);
	psy_ui_textarea_init_single_line(&self->edit_credits, &self->credits);	
	psy_ui_textarea_enable_input_field(&self->edit_credits);
	psy_ui_component_set_align(&self->edit_credits.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_margin(&self->edit_credits.component, margin);
	psy_signal_connect(&self->edit_credits.signal_accept, self,
		songpropertiesview_oneditaccept);
	psy_signal_connect(&self->edit_credits.signal_reject, self,
		songpropertiesview_oneditreject);
	psy_signal_connect(&self->edit_credits.component.signal_keydown, self,
		songpropertiesview_onfilterkeys);
	/* SamplerIndex */
	intedit_init_connect(&self->samplerindex, &self->top,
		"Sampler Index", 0x3E, 0, 0x3F,
		self, songpropertiesview_onsamplerindexchange);
	/* Speed */
	psy_ui_component_init(&self->speed, &self->top, NULL);
	psy_ui_component_set_default_align(&self->speed, psy_ui_ALIGN_LEFT,
		margin);
	psy_ui_label_init_text(&self->label_speed, &self->speed,
		"songproperties.speed");
	psy_ui_component_init(&self->speedbar, &self->speed, NULL);
	psy_ui_component_set_align(&self->speedbar, psy_ui_ALIGN_LEFT);
	psy_ui_label_set_char_number(&self->label_speed, charnum);
	psy_ui_component_set_default_align(&self->speedbar, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make(psy_ui_value_make_px(0),
			psy_ui_value_make_ew(1.0), psy_ui_value_make_px(0.0),
			psy_ui_value_make_px(0.0)));		
	/* Tempo */
	intedit_init_connect(&self->tempo, &self->speedbar, "songproperties.tempo",
		32, 32, 999, self, songpropertiesview_ontempoeditchange);
	intedit_init_connect(&self->lpb, &self->speedbar, "songproperties.lpb",
		1, 1, 32, self, songpropertiesview_onlpbeditchange);
	intedit_init_connect(&self->tpb, &self->speedbar, "songproperties.tpb",
		1, 1, 99, self, songpropertiesview_ontpbeditchange);
	intedit_init_connect(&self->etpb, &self->speedbar, "songproperties.etpb",
		0, 0, 99, self, songpropertiesview_onetpbeditchange);	
	/* Real Speed */
	psy_ui_label_init_text(&self->realtempo_desc, &self->speedbar,
		"songproperties.realtempo");
	psy_ui_label_init(&self->realtempo, &self->speedbar);
	psy_ui_label_set_char_number(&self->realtempo, 8);
	psy_ui_label_init_text(&self->realticksperbeat_desc, &self->speedbar,
		"songproperties.realtpb");	
	psy_ui_label_init(&self->realticksperbeat, &self->speedbar);
	psy_ui_label_set_char_number(&self->realticksperbeat, 8);			
	psy_ui_component_set_margin(&self->samplerindex.component, margin);
	/* Comments */
	psy_ui_component_init(&self->comments, &self->component, NULL);
	psy_ui_component_set_style_type(&self->comments,
		STYLE_SONGPROPERTIES_COMMENTS);
	psy_ui_component_set_align(&self->comments, psy_ui_ALIGN_CLIENT);
	psy_ui_label_init_text(&self->label_comments, &self->comments,
		"songproperties.extcomments");	
	psy_ui_label_set_textalignment(&self->label_comments,
		psy_ui_ALIGNMENT_TOP);	
	psy_ui_component_set_align(&self->label_comments.component,
		psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(&self->label_comments.component,
		psy_ui_margin_make_em(0.0, 2.0, 1.0, 3.0));
	psy_ui_textarea_init(&self->edit_comments, &self->comments);
	psy_ui_component_set_style_type(&self->edit_comments.component,
		STYLE_SONGPROPERTIES_COMMENTS_EDIT);		
	psy_ui_component_set_align(&self->edit_comments.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_margin(&self->edit_comments.component,
		psy_ui_margin_make_em(0.0, 2.0, 1.0, 3.0));
	psy_ui_component_set_align(&self->edit_comments.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->edit_comments.pane.signal_change, self,
		songpropertiesview_oncommentschanged);
	psy_signal_connect(&self->edit_comments.component.signal_keydown, self,
		songpropertiesview_onfilterkeys);
	songpropertiesview_read(self);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		songpropertiesview_onsongchanged);
	psy_signal_connect(&self->component.signal_hide, self,
		songpropertiesview_onhide);
	psy_signal_connect(&self->component.signal_keydown, self,
		songpropertiesview_on_key_down);
	psy_signal_connect(&self->component.signal_keyup, self,
		songpropertiesview_onkeyup);	
}

void songpropertiesview_read(SongPropertiesView* self)
{	
	if (self->song) {
		psy_ui_textarea_set_text(&self->edit_title,
			self->song->properties.title);
		psy_ui_textarea_set_text(&self->edit_credits,
			self->song->properties.credits);
		psy_ui_textarea_set_text(&self->edit_comments,
			self->song->properties.comments);
		intedit_set_value(&self->tempo, (int)self->song->properties.bpm);
		intedit_set_value(&self->lpb, (int)self->song->properties.lpb);
		intedit_set_value(&self->tpb, (int)self->song->properties.tpb);
		intedit_set_value(&self->etpb, (int)
			self->song->properties.extraticksperbeat);
		intedit_set_value(&self->samplerindex, (int)
			self->song->properties.samplerindex);
		songpropertiesview_updaterealspeed(self);
	}
}

void songpropertiesview_onsongchanged(SongPropertiesView* self,
	Workspace* sender)
{
	if (sender->song_has_file) {
		songpropertiesview_disableedit(self);
	} else {
		songpropertiesview_enableedit(self);
	}
	self->song = sender->song;
	songpropertiesview_read(self);
}

void songpropertiesview_enableedit(SongPropertiesView* self)
{
	psy_ui_textarea_enableedit(&self->edit_title);
	psy_ui_textarea_enableedit(&self->edit_credits);
	psy_ui_textarea_enableedit(&self->edit_comments);
	intedit_enable_edit(&self->tpb);
	intedit_enable_edit(&self->etpb);	
}

void songpropertiesview_disableedit(SongPropertiesView* self)
{
	psy_ui_textarea_preventedit(&self->edit_title);
	psy_ui_textarea_preventedit(&self->edit_credits);
	psy_ui_textarea_preventedit(&self->edit_comments);
	intedit_prevent_edit(&self->tpb);
	intedit_prevent_edit(&self->etpb);	
}

void songpropertiesview_onhide(SongPropertiesView* self,
	psy_ui_Component* sender)
{
	songpropertiesview_enableedit(self);
}

void songpropertiesview_oneditaccept(SongPropertiesView* self,
	psy_ui_TextArea* sender)
{
	if (self->song) {
		if (psy_strlen(psy_ui_textarea_text(sender)) == 0) {
			if (sender == &self->edit_title) {
				psy_ui_textarea_set_text(sender, "Untitled");
			}
		}
		if (sender == &self->edit_title) {
			psy_audio_song_settitle(self->song, psy_ui_textarea_text(sender));
		} else if (sender == &self->edit_credits) {
			psy_audio_song_setcredits(self->song, psy_ui_textarea_text(sender));
		}
	}
	psy_ui_component_set_focus(psy_ui_component_parent(&self->component));
}

void songpropertiesview_oneditreject(SongPropertiesView* self,
	psy_ui_TextArea* sender)
{	
	if (self->song) {
		if (sender == &self->edit_title) {
			psy_ui_textarea_set_text(&self->edit_title,
				psy_audio_song_title(self->song));
		} else if (sender == &self->edit_credits) {
			psy_ui_textarea_set_text(&self->edit_credits,
				psy_audio_song_credits(self->song));
		}
	} else {
		psy_ui_textarea_set_text(&self->edit_title, "");
	}	
	psy_ui_component_set_focus(psy_ui_component_parent(&self->component));
}

void songpropertiesview_onfilterkeys(SongPropertiesView* self,
	psy_ui_Component* sender, psy_ui_KeyboardEvent* ev)
{
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_RETURN && sender != &self->edit_comments.component) {
		psy_ui_component_set_focus(&self->component);		
		psy_ui_keyboardevent_prevent_default(ev);
	} else
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_ESCAPE) {
		psy_ui_component_set_focus(&self->component);
		psy_ui_keyboardevent_prevent_default(ev);
	}	
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

void songpropertiesview_onsamplerindexchange(SongPropertiesView* self,
	IntEdit* sender)
{
	psy_audio_player_set_sampler_index(workspace_player(self->workspace), (uintptr_t)
		intedit_value(sender));	
}

void songpropertiesview_oncommentschanged(SongPropertiesView* self,
	psy_ui_Component* sender)
{
	psy_strreset(&self->song->properties.comments,
		psy_ui_textarea_text(&self->edit_comments));
}

void songpropertiesview_on_key_down(SongPropertiesView* self,
	psy_ui_Component* sender, psy_ui_KeyboardEvent* ev)
{
	if (songpropertiesview_haseditfocus(self)) {		
		psy_ui_keyboardevent_stop_propagation(ev);
	}
}

bool songpropertiesview_haseditfocus(SongPropertiesView* self)
{
	return (psy_ui_component_has_focus(&self->edit_comments.component) ||
		psy_ui_component_has_focus(&self->edit_credits.component) ||
		psy_ui_component_has_focus(&self->edit_title.component));
		/* psy_ui_component_has_focus(&self->edit_tpb.component)); */		
}

void songpropertiesview_onkeyup(SongPropertiesView* self,
	psy_ui_Component* sender, psy_ui_KeyboardEvent* ev)
{
	psy_ui_keyboardevent_stop_propagation(ev);
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
	psy_ui_label_set_text(&self->realtempo, text);
	psy_snprintf(text, 128, "%d", (int)
		songpropertiesview_realtpb(self));
	psy_ui_label_set_text(&self->realticksperbeat, text);
}
