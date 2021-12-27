/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(VUBAR_H)
#define VUBAR_H

/* host */
#include "clipbox.h"
#include "volslider.h"
#include "vumeter.h"
#include "workspace.h"

/*
** VuBar
**
** Composite of Vumeter, VolumeSlider and ClipBox. Displays and controls the
** Master volume and displays a clip warning if the amp range overflows
**
** Structure:
** psy_ui_ComponentImp
**          ^
**          |
**        VuBar <>------ Vumeter
**               |------ VolSlider
**               |------ ClipBox
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VuBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	Vumeter vumeter;
	VolSlider volslider;
	ClipBox clipbox;	
} VuBar;

void vubar_init(VuBar*, psy_ui_Component* parent, Workspace*);

void vubar_reset(VuBar*);
void vubar_idle(VuBar*);

INLINE psy_ui_Component* vubar_base(VuBar* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* VUBAR_H */
