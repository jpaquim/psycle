// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SONGTRACKBAR_H)
#define SONGTRACKBAR_H

#include "uicombobox.h"
#include "uilabel.h"
#include "workspace.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_Label headerlabel;
	psy_ui_ComboBox trackbox;	
	Workspace* workspace;
} SongTrackBar;

void songtrackbar_init(SongTrackBar*, psy_ui_Component* parent, Workspace*);

#endif
