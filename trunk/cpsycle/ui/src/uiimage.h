// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIIMAGE_H)
#define UIIMAGE_H

#include "uicomponent.h"
#include "uibitmap.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_Bitmap bitmap;
	UiAlignment alignment;	
} psy_ui_Image;

void ui_image_init(psy_ui_Image*, psy_ui_Component* parent);
void ui_image_setbitmapalignment(psy_ui_Image*, UiAlignment);	

#endif
