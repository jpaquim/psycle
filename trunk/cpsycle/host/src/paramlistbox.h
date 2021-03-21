// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(PARAMLISTBOX_H)
#define PARAMLISTBOX_H

// host
#include "workspace.h"
#include "knobui.h"
// ui
#include <uilistbox.h>
// audio
#include <machine.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_ui_Component component;
	psy_ui_ListBox listbox;
	KnobUi knob;
	psy_audio_Machine* machine;
	psy_ui_Bitmap knobbitmap;
	Workspace* workspace;
} ParameterListBox;

void parameterlistbox_init(ParameterListBox*, psy_ui_Component* parent, psy_audio_Machine*,
	Workspace* workspace);
void parameterlistbox_setmachine(ParameterListBox*, psy_audio_Machine*);
intptr_t parameterlistbox_selected(ParameterListBox*);

#ifdef __cplusplus
}
#endif

#endif
