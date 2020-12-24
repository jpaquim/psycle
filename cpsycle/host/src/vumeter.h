// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(VUMETER_H)
#define VUMETER_H

// host
#include "workspace.h"
// ui
#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

// Vumeter
//
// The VU Meter displays the left and right master volume during playback.
// A timer polls the BufferMemory of the Master and updates the rms volume
//
// Structure:
//  psy_ui_ComponentImp
//          ^
//          |     
//       Vumeter < >---- Workspace < >---- Song <>---- Machines

typedef struct {
	psy_ui_Colour rms;
	psy_ui_Colour peak;
	psy_ui_Colour background;
	psy_ui_Colour border;
} VumeterSkin;

typedef struct Vumeter {
	// inherits
	psy_ui_Component component;
	// ui elements
	VumeterSkin skin;
	// internal data
	psy_dsp_amp_t leftavg;
	psy_dsp_amp_t rightavg;
	psy_dsp_amp_t l_log;
	psy_dsp_amp_t r_log;
	// references
	Workspace* workspace;
} Vumeter;

void vumeter_init(Vumeter*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* vumeter_base(Vumeter* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* VUMETER_H */
