// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(RECENTVIEW_H)
#define RECENTVIEW_H

#include "propertiesview.h"
#include <uibutton.h>
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

// PlaylistBar
// 
// Displays recently opened songs

// PlaylistBar
typedef struct PlaylistBar {
	// inherits
	psy_ui_Component component;
	// internal	
	psy_ui_Button clear;
	psy_ui_Button play;
	psy_ui_Button stop;
	psy_ui_Button del;
	psy_ui_Button up;
	psy_ui_Button down;
} PlaylistBar;

void playlistbar_init(PlaylistBar*, psy_ui_Component* parent);

INLINE psy_ui_Component* playlistbar_base(PlaylistBar* self)
{
	return &self->component;
}

// PlaylistView
typedef struct PlaylistView {
	// inherits
	psy_ui_Component component;
	// internal
	PlaylistBar bar;	
	PropertiesView view;
	Workspace* workspace;
	bool starting;
	bool running;
} PlaylistView;

void playlistview_init(PlaylistView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);

psy_Property* playlistview_currfiles(PlaylistView*);

INLINE psy_ui_Component* playlistview_base(PlaylistView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* RECENTVIEW_H */
