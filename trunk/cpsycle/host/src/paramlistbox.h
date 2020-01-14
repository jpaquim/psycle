// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PARAMLISTBOX_H)
#define PARAMLISTBOX_H

#include <uilistbox.h>
#include <uislider.h>
#include <machine.h>

typedef struct {
	psy_ui_Component component;
	ui_listbox listbox;
	psy_ui_Slider slider;
	psy_audio_Machine* machine;
} ParameterListBox;

void parameterlistbox_init(ParameterListBox*, psy_ui_Component* parent, psy_audio_Machine*);
void parameterlistbox_setmachine(ParameterListBox*, psy_audio_Machine*);
int parameterlistbox_selected(ParameterListBox*);

#endif
