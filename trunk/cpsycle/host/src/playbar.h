// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PLAYBAR_H)
#define PLAYBAR_H

#include "uibutton.h"
#include "workspace.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_Button loop;
	psy_ui_Button recordnotes;
	psy_ui_Button play;
	psy_ui_Button playsel;
	psy_ui_Button stop;	
	psy_audio_Player* player;
	Workspace* workspace;
} PlayBar;

void playbar_init(PlayBar*, psy_ui_Component* parent, Workspace*);

#endif
