// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "recentview.h"
#include <songio.h>
#include <string.h>

void recentbar_init(RecentBar* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_CLIENT,
		psy_ui_margin_make(
			psy_ui_value_makepx(0), psy_ui_value_makeew(0.5),
			psy_ui_value_makeeh(0.5), psy_ui_value_makeeh(0.5)));
	psy_ui_component_init(&self->client, &self->component);	
	psy_ui_component_setdefaultalign(&self->client, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_button_init(&self->up, &self->client);
	psy_ui_button_seticon(&self->up, psy_ui_ICON_UP);
	psy_ui_button_init(&self->down, &self->client);
	psy_ui_button_seticon(&self->down, psy_ui_ICON_DOWN);
	psy_ui_button_init_text(&self->play, &self->client, "Play");
	psy_ui_button_init_text(&self->stop, &self->client, "Stop");	
	psy_ui_button_init_text(&self->del, &self->client, "Delete");
	psy_ui_button_init_text(&self->clear, &self->client, "Clear");
}

static psy_Property* recentview_next(RecentView*);
static void recentview_onselected(RecentView*, PropertiesView* sender,
	psy_Property*);
static void recentview_ondelete(RecentView*, psy_ui_Button* sender);
static void recentview_onclear(RecentView*, psy_ui_Button* sender);
static void recentview_onplay(RecentView*, psy_ui_Button* sender);
static void recentview_onstop(RecentView*, psy_ui_Button* sender);
static void recentview_onmoveup(RecentView*, psy_ui_Button* sender);
static void recentview_onmovedown(RecentView*, psy_ui_Button* sender);
static void recentview_ontimer(RecentView*, uintptr_t timerid);
static void recentview_onmousedown(RecentView*, psy_ui_MouseEvent*);
static void recentview_onsongchanged(RecentView*, Workspace* sender,
	int flag, psy_audio_Song*);
// vtable
static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(RecentView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);		
		vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			recentview_onmousedown;
		vtable.ontimer = (psy_ui_fp_component_ontimer)
			recentview_ontimer;
	}
}

void recentview_init(RecentView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;	
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_BACKGROUND_NONE);
	self->workspace = workspace;
	recentbar_init(&self->bar, &self->component);
	psy_ui_component_setalign(&self->bar.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->bar.clear.signal_clicked, self,
		recentview_onclear);
	psy_signal_connect(&self->bar.play.signal_clicked, self,
		recentview_onplay);
	psy_signal_connect(&self->bar.stop.signal_clicked, self,
		recentview_onstop);
	psy_signal_connect(&self->bar.del.signal_clicked, self,
		recentview_ondelete);
	psy_signal_connect(&self->bar.up.signal_clicked, self,
		recentview_onmoveup);
	psy_signal_connect(&self->bar.down.signal_clicked, self,
		recentview_onmovedown);
	propertiesview_init(&self->view, &self->component, tabbarparent,
		workspace_recentsongs(workspace), workspace);
	self->view.renderer.showkeyselection = TRUE;
	propertiesview_setcolumnwidth(&self->view, 1.f, 0.f, 0.f);
	psy_ui_component_hide(&self->view.tabbar.component);
	propertiesrenderer_setfixedwidth(&self->view.renderer,
		psy_ui_value_makeew(40));	
	psy_ui_component_setalign(&self->view.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->view.signal_selected, self,
		recentview_onselected);
	self->starting = FALSE;
	psy_signal_connect(&workspace->signal_songchanged, self,
		recentview_onsongchanged);
}

void recentview_onselected(RecentView* self, PropertiesView* sender,
	psy_Property* property)
{
	if (psy_property_insection(property, self->workspace->playlist.recentfiles)) {
		if (!self->workspace->filename || strcmp(self->workspace->filename,
				psy_property_item_str(property)) != 0) {						
			workspace_loadsong(self->workspace, psy_property_item_str(property),
				generalconfig_playsongafterload(psycleconfig_general(
					workspace_conf(self->workspace))));			
			psy_ui_component_invalidate(&self->view.renderer.component);
		}
	}
}

void recentview_ondelete(RecentView* self, psy_ui_Button* sender)
{
	if (self->view.renderer.selected &&
			self->view.renderer.selected->parent ==
			self->workspace->playlist.recentfiles) {
		psy_property_remove(self->view.renderer.selected->parent,
			self->view.renderer.selected);
		self->view.renderer.selected = NULL;
		psy_playlist_save(&self->workspace->playlist);
		psy_ui_component_invalidate(&self->view.component);
	}
}

void recentview_onclear(RecentView* self, psy_ui_Button* sender)
{
	self->view.renderer.selected = NULL;
	workspace_clearrecentsongs(self->workspace);
	psy_ui_component_align(&self->view.component);
	psy_ui_component_invalidate(&self->view.component);
}

void recentview_onplay(RecentView* self, psy_ui_Button* sender)
{
	psy_audio_player_stop(workspace_player(self->workspace));
	self->starting = TRUE;
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void recentview_onstop(RecentView* self, psy_ui_Button* sender)
{
	psy_ui_component_stoptimer(&self->component, 0);
	self->starting = FALSE;
	psy_audio_player_stop(workspace_player(self->workspace));
}

void recentview_ontimer(RecentView* self, uintptr_t timerid)
{
	if (!psy_audio_player_playing(workspace_player(self->workspace))) {
		psy_Property* next;
		
		next = recentview_next(self);
		if (next) {						
			self->view.renderer.selected = next;
			workspace_loadsong(self->workspace, psy_property_item_str(next), FALSE);			
			psy_audio_sequencer_stoploop(&workspace_player(self->workspace)->sequencer);
			psy_audio_player_setposition(workspace_player(self->workspace), 0);
			psy_audio_player_start(workspace_player(self->workspace));
			self->starting = FALSE;
			psy_ui_component_invalidate(&self->view.renderer.component);
		} else {
			psy_audio_player_stop(workspace_player(self->workspace));
			psy_ui_component_stoptimer(&self->component, 0);
			psy_ui_component_invalidate(&self->view.renderer.component);
		}		
	}
}

psy_Property* recentview_next(RecentView* self)
{
	if (self->view.renderer.properties && self->view.renderer.selected) {
		psy_Property* parent;

		parent = self->view.renderer.selected->parent;
		if (parent) {
			psy_List* node;
			
			node = psy_list_findentry(parent->children, self->view.renderer.selected);
			if (node) {
				if (!self->starting) {
					psy_list_next(&node);
				}
				if (node) {
					return (psy_Property*)psy_list_entry(node);
				}
			}
		}
	}
	return NULL;
}

void recentview_onmousedown(RecentView* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 2) {
		psy_ui_component_hide_align(&self->component);
	}
}

void recentview_onsongchanged(RecentView* self, Workspace* sender,
	int flag, psy_audio_Song* song)
{	
	psy_ui_component_invalidate(&self->view.renderer.component);	
}

void recentview_onup(RecentView* self, psy_ui_Button* sender)
{

}

void recentview_onmoveup(RecentView* self, psy_ui_Button* sender)
{
	if (self->view.renderer.properties && self->view.renderer.selected) {
		psy_property_moveup(self->view.renderer.selected);
		psy_playlist_save(&self->workspace->playlist);
		psy_ui_component_invalidate(&self->view.renderer.component);
	}
}

void recentview_onmovedown(RecentView* self, psy_ui_Button* sender)
{
	if (self->view.renderer.properties && self->view.renderer.selected) {
		psy_property_movedown(self->view.renderer.selected);
		psy_playlist_save(&self->workspace->playlist);
		psy_ui_component_invalidate(&self->view.renderer.component);
	}
}
