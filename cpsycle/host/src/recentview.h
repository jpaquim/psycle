// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(RECENTVIEW_H)
#define RECENTVIEW_H

#include "settingsview.h"
#include <uibutton.h>
#include "workspace.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_Button clear;
	SettingsView view;
	Workspace* workspace;
} RecentView;

void recentview_init(RecentView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);

#endif /* RECENTVIEW_H */
