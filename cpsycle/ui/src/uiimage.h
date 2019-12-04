// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIIMAGE_H)
#define UIIMAGE_H

#include "uicomponent.h"
#include "uibitmap.h"

typedef struct {
	ui_component component;
	ui_bitmap bitmap;
	UiAlignment alignment;	
} ui_image;

void ui_image_init(ui_image*, ui_component* parent);
void ui_image_setbitmapalignment(ui_image*, UiAlignment);	

#endif
