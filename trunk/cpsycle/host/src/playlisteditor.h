// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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
	psy_ui_Component component;
	psy_ui_Component row1;
	psy_ui_Button addsong;
	psy_ui_Button removesong;
	psy_ui_Component row2;
	psy_ui_Button prev;
	psy_ui_Button play;
	psy_ui_Button stop;
	psy_ui_Button next;	
} PlayListEditorButtons;

void playlisteditorbuttons_init(PlayListEditorButtons*, psy_ui_Component* parent);

typedef struct PlayListEditor {
	psy_ui_Component component;
	psy_ui_Listbox listbox;
	PlayListEditorButtons buttons;
	psy_List* entries;
	Workspace* workspace;
	psy_List* currentry;
	psy_List* nextentry;
} PlayListEditor;

void playlisteditor_init(PlayListEditor*, psy_ui_Component* parent, Workspace*);

#endif