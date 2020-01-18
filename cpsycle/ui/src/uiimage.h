// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_IMAGE_H
#define psy_ui_IMAGE_H

#include "uicomponent.h"
#include "uibitmap.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_Bitmap bitmap;
	psy_ui_Alignment alignment;	
} psy_ui_Image;

void psy_ui_image_init(psy_ui_Image*, psy_ui_Component* parent);
void psy_ui_image_setbitmapalignment(psy_ui_Image*, psy_ui_Alignment);

#endif /* psy_ui_IMAGE_H */
