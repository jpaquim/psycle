/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MACHINEBAR_H)
#define MACHINEBAR_H

/* host */
#include "workspace.h"
/* ui */
#include "uibutton.h"
#include "uicombobox.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** Select Machine – Select the machine to be used
** Opens Gear Rack, CPU, MIDI, Lua Plugin Editor View
** Sampler : Select Instrument – Selects the instrument to be used or
** other Generator/Effect: Select Tweak Parameter
*/

typedef enum {
	MACHINEBAR_INST,
	MACHINEBAR_AUX,
	MACHINEBAR_PARAM
} MachineBarInstParamMode;

typedef struct MachineBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_ComboBox machinebox;	
	psy_ui_Button prevmachinebutton;
	psy_ui_Button nextmachinebutton;
	psy_ui_Button gear;
	psy_ui_Button dock;
	psy_ui_Button editor;
	psy_ui_Button cpu;
	psy_ui_Button midi;
	psy_ui_ComboBox selectinstparam;
	psy_ui_ComboBox instparambox;
	void (*selchange)(void*, int);
	void* eventcontext;
	psy_audio_Machines* machines;
	psy_audio_Instruments* instruments;
	int prevent_selchange_notify;
	psy_Table comboboxslots;
	psy_Table slotscombobox;
	/* references */
	Workspace* workspace;
} MachineBar;

void machinebar_init(MachineBar*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* machinebar_base(MachineBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MACHINEBAR_H */
