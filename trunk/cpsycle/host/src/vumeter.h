// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(VUMETER_H)
#define VUMETER_H

#include "workspace.h"

#include "uicomponent.h"

// aim: The VU Meter displays the left and right master volume during playback.
//      A timer polls the BufferMemory of the Master and updates the rms volume
//
// Structure:
//  psy_ui_ComponentImp
//          ^
//          |     
//       Vumeter < >---- Workspace < >---- Song <>---- Machines

typedef struct {
	psy_ui_Color rms;
	psy_ui_Color peak;
	psy_ui_Color background;
	psy_ui_Color border;
} VumeterSkin;

typedef struct {	
	psy_ui_Component component;
	VumeterSkin skin;
	psy_dsp_amp_t leftavg;
	psy_dsp_amp_t rightavg;
	Workspace* workspace;
} Vumeter;

void vumeter_init(Vumeter*, psy_ui_Component* parent, Workspace*);

#endif
