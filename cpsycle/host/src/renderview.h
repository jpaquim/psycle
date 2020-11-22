// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(RENDERVIEW_H)
#define RENDERVIEW_H

#include "propertiesview.h"

#include <uicomponent.h>
#include <uilabel.h>
#include <uiedit.h>
#include <uibutton.h>
#include <uicheckbox.h>
#include "workspace.h"

// aim: View for saving a song as a Windows PCM Wav.

typedef struct {
	psy_ui_Component component;
	PropertiesView view;
	psy_Property* properties;
	Workspace* workspace;
	psy_AudioDriver* fileoutdriver;
	psy_AudioDriver* curraudiodriver;	
	int restoreloopmode;
	psy_dsp_DitherSettings restoredither;
	bool restoredodither;
} RenderView;

void renderview_init(RenderView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);

#endif
