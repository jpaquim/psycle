// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PLAYBAR_H)
#define PLAYBAR_H

#include "uicomponent.h"
#include "list.h"

typedef struct {
	List* tabs;
	ui_component component;
	int selected;
	Signal signal_play;
	Signal signal_stop;
} PlayBar;

void InitPlayBar(PlayBar*, ui_component* parent);
void playbar_append(PlayBar*, const char* label);
void playbar_select(PlayBar*, int tab);

#endif
