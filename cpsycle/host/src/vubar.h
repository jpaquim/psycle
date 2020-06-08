// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(VUBAR_H)
#define VUBAR_H

#include "clipbox.h"
#include "volslider.h"
#include "vumeter.h"
#include "workspace.h"

// aim: The VU Bar combines a Vumeter, a VolumeSlider and a ClipBox
//      to display and control the Master volume
//
// Structure:
//  psy_ui_ComponentImp
//          ^
//          |     
//        VuBar <>------ Vumeter
//               |------ VolSlider
//               |------ ClipBox

typedef struct {
	psy_ui_Component component;
	Vumeter vumeter;
	VolSlider volslider;
	ClipBox clipbox;	
} VuBar;

void vubar_init(VuBar*, psy_ui_Component* parent, Workspace*);

#endif
