// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PLAYBAR_H)
#define PLAYBAR_H

// host
#include "workspace.h"
// ui
#include "uibutton.h"
#include "uicombobox.h"
#include "uiedit.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PlayBar {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Button loop;
	psy_ui_ComboBox playmode;	
	psy_ui_Edit loopbeatsedit;	
	psy_ui_Button loopbeatsless;
	psy_ui_Button loopbeatsmore;
	psy_ui_Button recordnotes;
	psy_ui_Button play;	
	psy_ui_Button stop;
	// references
	psy_audio_Player* player;
	Workspace* workspace;
} PlayBar;

void playbar_init(PlayBar*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* playbar_base(PlayBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PLAYBAR_H */
