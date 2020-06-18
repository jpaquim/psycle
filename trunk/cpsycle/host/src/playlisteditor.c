// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "playlisteditor.h"
#include <dir.h>
#include <exclusivelock.h>
#include <songio.h>

#include <uiopendialog.h>

#include <string.h>
#include <stdlib.h>

#define TIMERID_PLAYLIST 4000

static void playlisteditor_ontimer(PlayListEditor*, uintptr_t timerid);

static void playlisteditor_onaddsong(PlayListEditor*, psy_ui_Component* sender);
static void playlisteditor_onremovesong(PlayListEditor*, psy_ui_Component* sender);
static void playlisteditor_onplay(PlayListEditor*, psy_ui_Component* sender);
static void playlisteditor_onstop(PlayListEditor*, psy_ui_Component* sender);
static void playlisteditor_onprev(PlayListEditor*, psy_ui_Component* sender);
static void playlisteditor_onnext(PlayListEditor*, psy_ui_Component* sender);
static void playlisteditor_onlistchanged(PlayListEditor*, psy_ui_Component* sender,
	int slot);
static void playlisteditor_buildplaylist(PlayListEditor*);
static void playlisteditor_onpreferredsize(PlayListEditor*, psy_ui_Size* limit,
	psy_ui_Size* rv);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(PlayListEditor* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			playlisteditor_onpreferredsize;
		vtable.ontimer = (psy_ui_fp_ontimer)
			playlisteditor_ontimer;
	}
}

void playlistentry_init(PlayListEntry* self, const char* title, const char* path)
{
	self->title = strdup(title);
	self->path = strdup(path);
}

void playlistentry_dispose(PlayListEntry* self)
{
	free(self->title);
	free(self->path);
}

PlayListEntry* playlistentry_alloc(void)
{
	return (PlayListEntry*) malloc(sizeof(PlayListEntry));

}

PlayListEntry* playlistentry_allocinit(const char* title, const char* path)
{
	PlayListEntry* rv;

	rv = playlistentry_alloc();
	if (rv) {
		playlistentry_init(rv, title, path);
	}
	return rv;
}

void playlisteditorbuttons_init(PlayListEditorButtons* self,
	psy_ui_Component* parent)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makeeh(0.5),
		psy_ui_value_makeew(0.5), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_init(&self->row1, &self->component);
	psy_ui_component_enablealign(&self->row1);
	psy_ui_component_setalign(&self->row1, psy_ui_ALIGN_TOP);
	psy_ui_button_init(&self->addsong, &self->row1);
	psy_ui_button_settext(&self->addsong, "+ Song");
	psy_ui_button_init(&self->removesong, &self->row1);
	psy_ui_button_settext(&self->removesong, "- Song");
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->row1, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
			&margin));	
	psy_ui_component_init(&self->row2, &self->component);
	psy_ui_component_enablealign(&self->row2);
	psy_ui_component_setalign(&self->row2, psy_ui_ALIGN_TOP);
	psy_ui_button_init(&self->prev, &self->row2);
	psy_ui_button_settext(&self->prev, "Prev");
	psy_ui_button_init(&self->play, &self->row2);
	psy_ui_button_settext(&self->play, "Play");
	psy_ui_button_init(&self->stop, &self->row2);
	psy_ui_button_settext(&self->stop, "Stop");
	psy_ui_button_init(&self->next, &self->row2);
	psy_ui_button_settext(&self->next, "Next");
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->row2, 0),
		psy_ui_ALIGN_LEFT,
			&margin));	
}

void playlisteditor_init(PlayListEditor* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	self->workspace = workspace;
	self->entries = 0;
	self->currentry = 0;
	self->nextentry = 0;
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_enablealign(&self->component);
	psy_ui_listbox_init(&self->listbox, &self->component);	
	psy_ui_component_setalign(&self->listbox.component, psy_ui_ALIGN_CLIENT);
	playlisteditorbuttons_init(&self->buttons, &self->component);
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_BOTTOM);	
	psy_signal_connect(&self->buttons.addsong.signal_clicked, self, 
		playlisteditor_onaddsong);
	psy_signal_connect(&self->buttons.removesong.signal_clicked, self, 
		playlisteditor_onremovesong);
	psy_signal_connect(&self->buttons.play.signal_clicked, self, 
		playlisteditor_onplay);
	psy_signal_connect(&self->buttons.stop.signal_clicked, self, 
		playlisteditor_onstop);
	psy_signal_connect(&self->buttons.prev.signal_clicked, self, 
		playlisteditor_onprev);
	psy_signal_connect(&self->buttons.next.signal_clicked, self, 
		playlisteditor_onnext);	
	psy_signal_connect(&self->listbox.signal_selchanged, self,
		playlisteditor_onlistchanged);
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make(psy_ui_value_makeew(20),
		psy_ui_value_makeeh(12)));
}

void playlisteditor_onaddsong(PlayListEditor* self, psy_ui_Component* sender)
{
	psy_ui_OpenDialog dialog;	

	psy_ui_opendialog_init_all(&dialog, 0, "Load Song",
		psy_audio_songfile_loadfilter(), "PSY",
		workspace_songs_directory(self->workspace));
	if (psy_ui_opendialog_execute(&dialog)) {	
		char name[4096];
		char prefix[4096];
		char ext[4096];
		PlayListEntry* entry;
		int currsel;

		currsel = psy_ui_listbox_cursel(&self->listbox);
		psy_dir_extract_path(psy_ui_opendialog_filename(&dialog), prefix, name, ext);
		entry = playlistentry_allocinit(name, psy_ui_opendialog_filename(&dialog));
		if (entry) {
			psy_List* p;

			p = psy_list_append(&self->entries, entry);
			playlisteditor_buildplaylist(self);
			if (!self->currentry) {				
				psy_ui_listbox_setcursel(&self->listbox, 0);
				self->currentry = p;
				self->nextentry = p;
			} else {				
				psy_ui_listbox_setcursel(&self->listbox, currsel);
			}			
		}
	}
	psy_ui_opendialog_dispose(&dialog);
}

void playlisteditor_onremovesong(PlayListEditor* self, psy_ui_Component* sender)
{
	
}

void playlisteditor_buildplaylist(PlayListEditor* self)
{
	psy_List* p;
	
	psy_ui_listbox_clear(&self->listbox);
	for (p = self->entries; p != NULL; p = p->next) {
		PlayListEntry* entry;

		entry = (PlayListEntry*) p->entry;
		psy_ui_listbox_addtext(&self->listbox, entry->title);
	}	
}

void playlisteditor_onplay(PlayListEditor* self, psy_ui_Component* sender)
{		
	psy_audio_player_stop(&self->workspace->player);
	self->nextentry = self->currentry;
	psy_ui_component_starttimer(&self->component, TIMERID_PLAYLIST, 50);
}

void playlisteditor_onstop(PlayListEditor* self, psy_ui_Component* sender)
{	
	psy_ui_component_stoptimer(&self->component, TIMERID_PLAYLIST);
	psy_audio_player_stop(&self->workspace->player);
}

void playlisteditor_onprev(PlayListEditor* self, psy_ui_Component* sender)
{
	if (self->currentry->prev) {
		self->nextentry = self->currentry->prev;
		psy_ui_listbox_setcursel(&self->listbox,
			psy_ui_listbox_cursel(&self->listbox) - 1);
		psy_audio_player_stop(&self->workspace->player);
	}
}

void playlisteditor_onnext(PlayListEditor* self, psy_ui_Component* sender)
{
	if (self->currentry->next) {
		self->nextentry = self->currentry->next;
		psy_ui_listbox_setcursel(&self->listbox,
			psy_ui_listbox_cursel(&self->listbox) + 1);
		psy_audio_player_stop(&self->workspace->player);
	}
}

void playlisteditor_ontimer(PlayListEditor* self, uintptr_t timerid)
{		
	if (!psy_audio_player_playing(&self->workspace->player)) {
		if (self->nextentry) {
			PlayListEntry* entry;
			psy_List* p;
			int c;
			
			self->currentry = self->nextentry;
			entry = (PlayListEntry*) self->currentry->entry;			
			workspace_loadsong(self->workspace, entry->path, FALSE);		
			psy_audio_sequencer_stoploop(&self->workspace->player.sequencer);
			psy_audio_player_setposition(&self->workspace->player, 0);
			psy_audio_player_start(&self->workspace->player);
			self->nextentry = self->currentry->next;
			
			p = self->entries;
			c = 0;
			while (p != NULL) {
				if (p == self->currentry) {
					break;
				}
				p = p->next;
				++c;
			}
			psy_ui_listbox_setcursel(&self->listbox, c);
		} else {
			psy_audio_player_stop(&self->workspace->player);
			psy_ui_component_stoptimer(&self->component, TIMERID_PLAYLIST);
		}
	}
}

void playlisteditor_onlistchanged(PlayListEditor* self, psy_ui_Component* sender,
	int slot)
{
	psy_List* p;
	
	p = psy_list_at(self->entries, slot);
	psy_audio_player_stop(&self->workspace->player);
	self->currentry = p;
	self->nextentry = p;
}

void playlisteditor_onpreferredsize(PlayListEditor* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	if (rv) {
		*rv = psy_ui_component_size(&self->component);
	}
}
