// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiimage.h"

static void ondestroy(psy_ui_Image*);
static void ondraw(psy_ui_Image*, psy_ui_Component* sender, psy_ui_Graphics* g);
static int checkalignment(psy_ui_Image*, psy_ui_Alignment alignment);

void psy_ui_image_init(psy_ui_Image* self, psy_ui_Component* parent)
{  
    psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_bitmap_init(&self->bitmap);
	psy_signal_connect(&self->component.signal_draw, self, ondraw);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	self->alignment = psy_ui_ALIGNMENT_CENTER_VERTICAL;
}

void psy_ui_image_init_resource(psy_ui_Image* self, psy_ui_Component* parent,
	uintptr_t resourceid)
{
	psy_ui_image_init(self, parent);
	psy_ui_bitmap_loadresource(&self->bitmap, resourceid);	
}

void psy_ui_image_init_resource_transparency(psy_ui_Image* self,
	psy_ui_Component* parent, uintptr_t resourceid, psy_ui_Colour transparency)
{
	psy_ui_image_init_resource(self, parent, resourceid, transparency);
	psy_ui_bitmap_settransparency(&self->bitmap, transparency);
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
	psy_ui_RealSize bmpsize;
	const psy_ui_TextMetric* tm;
	double x;
	double y;
	double width;
	double height;	

	size = psy_ui_component_size(&self->component);	
	tm = psy_ui_component_textmetric(&self->component);
	width = psy_ui_value_px(&size.width, tm);
	height = psy_ui_value_px(&size.height, tm);
	bmpsize = psy_ui_bitmap_size(&self->bitmap);	
	if (checkalignment(self, psy_ui_ALIGNMENT_CENTER_HORIZONTAL)) {
		x = (width - bmpsize.width) / 2;
	} else 		
	if (checkalignment(self, psy_ui_ALIGNMENT_RIGHT)) {
		x = width - bmpsize.width;
	} else {		
		x = 0;		
	}
	if (checkalignment(self, psy_ui_ALIGNMENT_CENTER_VERTICAL)) {								
		y = (height - bmpsize.height) / 2;
	} else 		
	if (checkalignment(self, psy_ui_ALIGNMENT_BOTTOM)) {
		y = height - bmpsize.height;
	} else {		
		y = 0;		
	}	
	psy_ui_drawbitmap(g, &self->bitmap,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(x, y), bmpsize),
		psy_ui_realpoint_zero());
}

int checkalignment(psy_ui_Image* self, psy_ui_Alignment alignment)
{
	return (self->alignment & alignment) == alignment;	
}
