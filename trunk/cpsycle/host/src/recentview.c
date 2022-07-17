/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "recentview.h"
/* host */
#include "styles.h"
/* audio */
#include <exclusivelock.h>
#include <songio.h>


/* implementation */
void playlistbar_init(PlaylistBar* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_padding(&self->component,
		psy_ui_margin_make_em(0.0, 0.0, 0.5, 0.0));
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init(&self->up, &self->component);
	psy_ui_button_set_icon(&self->up, psy_ui_ICON_UP);
	psy_ui_button_init(&self->down, &self->component);
	psy_ui_button_set_icon(&self->down, psy_ui_ICON_DOWN);
	psy_ui_button_init_text(&self->play, &self->component, "playlist.play");
	psy_ui_button_init_text(&self->stop, &self->component, "playlist.stop");
	psy_ui_button_init_text(&self->del, &self->component, "playlist.delete");
	psy_ui_button_init_text(&self->clear, &self->component, "playlist.clear");
}

/* PlaylistView */
/* prototypes */
static psy_Property* playlistview_next(PlaylistView*);
static void playlistview_onselected(PlaylistView*, PropertiesView* sender,
	psy_Property*);
static void playlistview_ondelete(PlaylistView*, psy_ui_Button* sender);
static void playlistview_onclear(PlaylistView*, psy_ui_Button* sender);
static void playlistview_onplay(PlaylistView*, psy_ui_Button* sender);
static void playlistview_onstop(PlaylistView*, psy_ui_Button* sender);
static void playlistview_onmoveup(PlaylistView*, psy_ui_Button* sender);
static void playlistview_onmovedown(PlaylistView*, psy_ui_Button* sender);
static void playlistview_on_timer(PlaylistView*, uintptr_t timerid);
static void playlistview_on_mouse_down(PlaylistView*, psy_ui_MouseEvent*);
static void playlistview_onsongchanged(PlaylistView*, Workspace* sender);
static void playlistview_onplaylistchanged(PlaylistView*, psy_Playlist* sender);
/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(PlaylistView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);		
		vtable.on_mouse_down = (psy_ui_fp_component_on_mouse_event)
			playlistview_on_mouse_down;
		vtable.on_timer = (psy_ui_fp_component_on_timer)
			playlistview_on_timer;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}
/* implementation */
void playlistview_init(PlaylistView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);	
	self->workspace = workspace;
	self->playlist = &workspace->playlist;
	playlistbar_init(&self->bar, &self->component);	
	psy_ui_component_set_align(&self->bar.component, psy_ui_ALIGN_TOP);	
	psy_signal_connect(&self->bar.clear.signal_clicked, self,
		playlistview_onclear);
	psy_signal_connect(&self->bar.play.signal_clicked, self,
		playlistview_onplay);
	psy_signal_connect(&self->bar.stop.signal_clicked, self,
		playlistview_onstop);
	psy_signal_connect(&self->bar.del.signal_clicked, self,
		playlistview_ondelete);
	psy_signal_connect(&self->bar.up.signal_clicked, self,
		playlistview_onmoveup);
	psy_signal_connect(&self->bar.down.signal_clicked, self,
		playlistview_onmovedown);
	propertiesview_init(&self->view, &self->component,
		tabbarparent, workspace_recentsongs(workspace), 1, FALSE,
		&workspace->inputhandler);	
	propertiesrenderer_set_style(&self->view.renderer,
		STYLE_RECENTVIEW_MAINSECTION,
		STYLE_RECENTVIEW_MAINSECTIONHEADER,
		STYLE_RECENTVIEW_LINE,
		STYLE_RECENTVIEW_LINE_HOVER,
		STYLE_RECENTVIEW_LINE_SELECT);
	psy_ui_component_set_preferred_size(&self->view.component,
		psy_ui_size_make_em(50.0, 0.0));
	psy_ui_component_hide(&self->view.tabbar.component);	
	psy_ui_component_set_align(&self->view.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->view.signal_selected, self,
		playlistview_onselected);
	self->starting = FALSE;
	self->running = FALSE;
	psy_signal_connect(&workspace->signal_songchanged, self,
		playlistview_onsongchanged);
	psy_signal_connect(&workspace->playlist.signal_changed, self,
		playlistview_onplaylistchanged);
}

void playlistview_onselected(PlaylistView* self, PropertiesView* sender,
	psy_Property* property)
{	
	if (psy_property_in_section(property, playlistview_currfiles(self))) {
		if (!self->workspace->song || self->starting || !self->workspace->song->filename ||
				strcmp(self->workspace->song->filename, psy_property_item_str(property)) != 0) {						
			workspace_load_song(self->workspace, psy_property_item_str(property),
				generalconfig_playsongafterload(psycleconfig_general(
					workspace_conf(self->workspace))));			
			psy_audio_exclusivelock_enter();
			psy_audio_sequencer_stop_loop(&workspace_player(self->workspace)->sequencer);
			psy_audio_player_setposition(workspace_player(self->workspace), 0);
			psy_audio_player_start(workspace_player(self->workspace));
			self->running = TRUE;
			psy_audio_exclusivelock_leave();			
		}
	}
}

psy_Property* playlistview_currfiles(PlaylistView* self)
{
	if (self->playlist) {
		return self->playlist->recentfiles;
	}
	return NULL;
}

void playlistview_ondelete(PlaylistView* self, psy_ui_Button* sender)
{
	if (propertiesview_selected(&self->view) &&
			psy_property_parent(propertiesview_selected(&self->view)) ==
			playlistview_currfiles(self)) {
		psy_property_remove(
			psy_property_parent(propertiesview_selected(&self->view)),
			propertiesview_selected(&self->view));
		psy_playlist_save(workspace_playlist(self->workspace));
		propertiesview_reload(&self->view);
	}
}

void playlistview_onclear(PlaylistView* self, psy_ui_Button* sender)
{	
	if (self->playlist) {
		psy_playlist_clear(self->playlist);
		propertiesview_reload(&self->view);
	}
}

void playlistview_onplay(PlaylistView* self, psy_ui_Button* sender)
{
	psy_audio_player_stop(workspace_player(self->workspace));
	self->starting = TRUE;	
	psy_ui_component_start_timer(&self->component, 0, 50);
}

void playlistview_onstop(PlaylistView* self, psy_ui_Button* sender)
{
	psy_ui_component_stop_timer(&self->component, 0);
	self->starting = FALSE;
	self->running = FALSE;
	psy_ui_button_disable_highlight(&self->bar.play);
	psy_audio_player_stop(workspace_player(self->workspace));
}

void playlistview_on_timer(PlaylistView* self, uintptr_t timerid)
{
	if (!psy_audio_player_playing(workspace_player(self->workspace))) {
		psy_Property* next;
		
		next = playlistview_next(self);
		if (next) {
			propertiesview_select(&self->view, next);								
			self->starting = FALSE;			
		} else {
			psy_audio_player_stop(workspace_player(self->workspace));
			psy_ui_component_stop_timer(&self->component, 0);
			self->running = FALSE;			
		}	
	}
	if (self->running) {
		psy_ui_button_highlight(&self->bar.play);
	} else {
		psy_ui_button_disable_highlight(&self->bar.play);
	}
}

psy_Property* playlistview_next(PlaylistView* self)
{
	if (propertiesview_selected(&self->view)) {
		if (!self->starting) {
			return psy_property_next(propertiesview_selected(&self->view));
		} else {
			return propertiesview_selected(&self->view);
		}		
	}
	return NULL;
}

void playlistview_on_mouse_down(PlaylistView* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_button(ev) == 2) {
		psy_ui_component_hide_align(&self->component);
	}
}

void playlistview_onsongchanged(PlaylistView* self, Workspace* sender)
{
	if (!workspace_song_has_file(sender)) {
		propertiesview_mark(&self->view, NULL);
	}
}

void playlistview_onplaylistchanged(PlaylistView* self, psy_Playlist* sender)
{
	propertiesview_reload(&self->view);	
}

void playlistview_onmoveup(PlaylistView* self, psy_ui_Button* sender)
{
	psy_Property* selected;

	selected = propertiesview_selected(&self->view);
	if (selected) {		
		psy_property_moveup(selected);
		psy_playlist_save(&self->workspace->playlist);
		propertiesview_reload(&self->view);		
		propertiesview_mark(&self->view, selected);		
	}
}

void playlistview_onmovedown(PlaylistView* self, psy_ui_Button* sender)
{
	psy_Property* selected;

	selected = propertiesview_selected(&self->view);
	if (selected) {
		psy_property_movedown(selected);		
		psy_playlist_save(&self->workspace->playlist);
		propertiesview_reload(&self->view);
		propertiesview_mark(&self->view, selected);
	}
}
