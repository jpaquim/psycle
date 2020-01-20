// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiimage.h"

static void ondestroy(psy_ui_Image*);
static void ondraw(psy_ui_Image*, psy_ui_Component* sender, psy_ui_Graphics* g);
static int checkalignment(psy_ui_Image*, psy_ui_Alignment alignment);

void psy_ui_image_init(psy_ui_Image* self, psy_ui_Component* parent)
{  
    psy_ui_component_init(&self->component, parent);
	psy_ui_bitmap_init(&self->bitmap);
	psy_signal_connect(&self->component.signal_draw, self, ondraw);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	self->alignment = psy_ui_ALIGNMENT_NONE;	
}

void ondestroy(psy_ui_Image* self)
{
	psy_ui_bitmap_dispose(&self->bitmap);
}

void psy_ui_image_setbitmapalignment(psy_ui_Image* self,
	psy_ui_Alignment alignment)
{
	self->alignment = alignment;
}

void ondraw(psy_ui_Image* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{
	psy_ui_Size size;
	psy_ui_Size bmpsize;
	int x;
	int y;
	
	size = psy_ui_component_size(&self->component);	
	bmpsize = psy_ui_bitmap_size(&self->bitmap);
	if (checkalignment(self, psy_ui_ALIGNMENT_CENTER_HORIZONTAL)) {
		x = (size.width - bmpsize.width) / 2;
	} else 		
	if (checkalignment(self, psy_ui_ALIGNMENT_RIGHT)) {
		x = size.width - bmpsize.width;		
	} else {		
		x = 0;		
	}
	if (checkalignment(self, psy_ui_ALIGNMENT_CENTER_VERTICAL)) {								
		y = (size.height - bmpsize.height) / 2;
	} else 		
	if (checkalignment(self, psy_ui_ALIGNMENT_BOTTOM)) {
		y = size.height - bmpsize.height;
	} else {		
		y = 0;		
	}	
	psy_ui_drawbitmap(g, &self->bitmap, 
		x, y, bmpsize.width, bmpsize.height, 0, 0);
}

int checkalignment(psy_ui_Image* self, psy_ui_Alignment alignment)
{
	return (self->alignment & alignment) == alignment;	
}
