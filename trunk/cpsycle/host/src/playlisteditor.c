// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "playlisteditor.h"
#include <string.h>
#include <dir.h>
#include <exclusivelock.h>

#define TIMERID_PLAYLIST 4000

static void playlisteditor_ontimer(PlayListEditor*, ui_component* sender, int timerid);

static void playlisteditor_onaddsong(PlayListEditor*, ui_component* sender);
static void playlisteditor_onremovesong(PlayListEditor*, ui_component* sender);
static void playlisteditor_onplay(PlayListEditor*, ui_component* sender);
static void playlisteditor_onstop(PlayListEditor*, ui_component* sender);
static void playlisteditor_onprev(PlayListEditor*, ui_component* sender);
static void playlisteditor_onnext(PlayListEditor*, ui_component* sender);
static void playlisteditor_onlistchanged(PlayListEditor*, ui_component* sender,
	int slot);
static void playlisteditor_buildplaylist(PlayListEditor*);

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
	ui_component* parent)
{
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makeeh(0.5), ui_value_makeew(0.5),
		ui_value_makepx(0), ui_value_makepx(0));
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_init(&self->row1, &self->component);
	ui_component_enablealign(&self->row1);
	ui_component_setalign(&self->row1, UI_ALIGN_TOP);
	ui_button_init(&self->addsong, &self->row1);
	ui_button_settext(&self->addsong, "+ Song");
	ui_button_init(&self->removesong, &self->row1);
	ui_button_settext(&self->removesong, "- Song");
	list_free(ui_components_setalign(
		ui_component_children(&self->row1, 0),
		UI_ALIGN_LEFT,
			&margin));	
	ui_component_init(&self->row2, &self->component);
	ui_component_enablealign(&self->row2);
	ui_component_setalign(&self->row2, UI_ALIGN_TOP);
	ui_button_init(&self->prev, &self->row2);
	ui_button_settext(&self->prev, "Prev");
	ui_button_init(&self->play, &self->row2);
	ui_button_settext(&self->play, "Play");
	ui_button_init(&self->stop, &self->row2);
	ui_button_settext(&self->stop, "Stop");
	ui_button_init(&self->next, &self->row2);
	ui_button_settext(&self->next, "Next");
	list_free(ui_components_setalign(
		ui_component_children(&self->row2, 0),
		UI_ALIGN_LEFT,
			&margin));	
}

void playlisteditor_init(PlayListEditor* self, ui_component* parent,
	Workspace* workspace)
{
	self->workspace = workspace;
	self->entries = 0;
	self->currentry = 0;
	self->nextentry = 0;
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);
	ui_listbox_init(&self->listbox, &self->component);	
	ui_component_setalign(&self->listbox.component, UI_ALIGN_CLIENT);
	playlisteditorbuttons_init(&self->buttons, &self->component);
	ui_component_setalign(&self->buttons.component, UI_ALIGN_BOTTOM);	
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
	ui_component_resize(&self->component, 150, 0);
	psy_signal_connect(&self->component.signal_timer, self,
		playlisteditor_ontimer);
	psy_signal_connect(&self->listbox.signal_selchanged, self,
		playlisteditor_onlistchanged);
	ui_component_resize(&self->component, 200, 200);
}

void playlisteditor_onaddsong(PlayListEditor* self, ui_component* sender)
{
	char path[MAX_PATH]	 = "";
	char title[MAX_PATH]	 = ""; 					
	static char filter[] = "All Songs (*.psy *.xm *.it *.s3m *.mod)" "\0*.psy;*.xm;*.it;*.s3m;*.mod\0"
				"Songs (*.psy)"				        "\0*.psy\0"
				"FastTracker II Songs (*.xm)"       "\0*.xm\0"
				"Impulse Tracker Songs (*.it)"      "\0*.it\0"
				"Scream Tracker Songs (*.s3m)"      "\0*.s3m\0"
				"Original Mod Format Songs (*.mod)" "\0*.mod\0";
	char  defaultextension[] = "PSY";	

	int showsonginfo = 0;	
	*path = '\0'; 
	if (ui_openfile(&self->component, title, filter, defaultextension, 
			workspace_songs_directory(self->workspace),
			path)) {		
		char name[4096];
		char prefix[4096];
		char ext[4096];
		PlayListEntry* entry;
		int currsel;

		currsel = ui_listbox_cursel(&self->listbox);
		extract_path(path, prefix, name, ext);
		entry = playlistentry_allocinit(name, path);
		if (entry) {
			List* p;

			p = list_append(&self->entries, entry);
			playlisteditor_buildplaylist(self);
			if (!self->currentry) {				
				ui_listbox_setcursel(&self->listbox, 0);
				self->currentry = p;
				self->nextentry = p;
			} else {				
				ui_listbox_setcursel(&self->listbox, currsel);
			}			
		}
	}
}

void playlisteditor_onremovesong(PlayListEditor* self, ui_component* sender)
{
	
}

void playlisteditor_buildplaylist(PlayListEditor* self)
{
	List* p;
	
	ui_listbox_clear(&self->listbox);
	for (p = self->entries; p != 0; p = p->next) {
		PlayListEntry* entry;

		entry = (PlayListEntry*) p->entry;
		ui_listbox_addstring(&self->listbox, entry->title);
	}	
}

void playlisteditor_onplay(PlayListEditor* self, ui_component* sender)
{		
	player_stop(&self->workspace->player);
	self->nextentry = self->currentry;
	ui_component_starttimer(&self->component, TIMERID_PLAYLIST, 50);
}

void playlisteditor_onstop(PlayListEditor* self, ui_component* sender)
{	
	ui_component_stoptimer(&self->component, TIMERID_PLAYLIST);
	player_stop(&self->workspace->player);
}

void playlisteditor_onprev(PlayListEditor* self, ui_component* sender)
{
	if (self->currentry->prev) {
		self->nextentry = self->currentry->prev;
		ui_listbox_setcursel(&self->listbox,
			ui_listbox_cursel(&self->listbox) - 1);
		player_stop(&self->workspace->player);
	}
}

void playlisteditor_onnext(PlayListEditor* self, ui_component* sender)
{
	if (self->currentry->next) {
		self->nextentry = self->currentry->next;
		ui_listbox_setcursel(&self->listbox,
			ui_listbox_cursel(&self->listbox) + 1);
		player_stop(&self->workspace->player);
	}
}

void playlisteditor_ontimer(PlayListEditor* self, ui_component* sender, int timerid)
{		
	if (!player_playing(&self->workspace->player)) {
		if (self->nextentry) {
			PlayListEntry* entry;
			List* p;
			int c;
			
			self->currentry = self->nextentry;
			entry = (PlayListEntry*) self->currentry->entry;			
			workspace_loadsong(self->workspace, entry->path);		
			sequencer_stoploop(&self->workspace->player.sequencer);					
			player_setposition(&self->workspace->player, 0);
			player_start(&self->workspace->player);
			self->nextentry = self->currentry->next;
			
			p = self->entries;
			c = 0;
			while (p != 0) {
				if (p == self->currentry) {
					break;
				}
				p = p->next;
				++c;
			}
			ui_listbox_setcursel(&self->listbox, c);
		} else {
			player_stop(&self->workspace->player);			
			ui_component_stoptimer(&self->component, TIMERID_PLAYLIST);		
		}
	}
}

void playlisteditor_onlistchanged(PlayListEditor* self, ui_component* sender,
	int slot)
{
	List* p;
	
	p = list_at(self->entries, slot);
	player_stop(&self->workspace->player);
	self->currentry = p;
	self->nextentry = p;
}
