// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SONGPROPERTIES_H)
#define SONGPROPERTIES_H

#include <uilabel.h>
#include <uiedit.h>
#include "workspace.h"

typedef struct {
	ui_component component;
	Song* song;
	ui_label label_title;
	ui_label label_credits;
	ui_label label_comments;
	ui_edit edit_title;
	ui_edit edit_credits;
	ui_edit edit_comments;	
} SongProperties;

void InitSongProperties(SongProperties* songproperties, ui_component* parent,
	Workspace*);

#endif
