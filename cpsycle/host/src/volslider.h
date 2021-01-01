// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(VOLSLIDER_H)
#define VOLSLIDER_H

// host
#include "workspace.h"
// audio
#include <machines.h>
// ui
#include <uislider.h>

#ifdef __cplusplus
extern "C" {
#endif

// Master Volume slider
//
// Controls the volume of the Master

typedef struct VolSlider {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Slider slider;
	// references
	Workspace* workspace;
} VolSlider;

void volslider_init(VolSlider*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* volslider_base(VolSlider* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* VOLSLIDER_H */
