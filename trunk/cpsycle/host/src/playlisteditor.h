// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PLAYLISTEDITOR_H)
#define PLAYLISTEDITOR_H

#include <uilistbox.h>
#include <uibutton.h>
#include "workspace.h"

typedef struct {
	char* title;
	char* path;
}  PlayListEntry;

void playlistentry_init(PlayListEntry*, const char* title, const char* path);
void playlistentry_dispose(PlayListEntry*);

PlayListEntry* playlistentry_alloc(void);
PlayListEntry* playlistentry_allocinit(const char* title, const char* path);

typedef struct {
	ui_component component;
	ui_component row1;
	ui_button addsong;
	ui_button removesong;
	ui_component row2;
	ui_button prev;
	ui_button play;
	ui_button stop;
	ui_button next;	
} PlayListEditorButtons;

void playlisteditorbuttons_init(PlayListEditorButtons*, ui_component* parent);

typedef struct PlayListEditor {
	ui_component component;
	ui_listbox listbox;
	PlayListEditorButtons buttons;
	psy_List* entries;
	Workspace* workspace;
	psy_List* currentry;
	psy_List* nextentry;
} PlayListEditor;

void playlisteditor_init(PlayListEditor*, ui_component* parent, Workspace*);

#endif