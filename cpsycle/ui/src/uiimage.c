// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiimage.h"

static void ondestroy(ui_image*);
static void ondraw(ui_image*, ui_component* sender, ui_graphics* g);
static int checkalignment(ui_image*, UiAlignment alignment);

void ui_image_init(ui_image* self, ui_component* parent)
{  
    ui_component_init(&self->component, parent);
	ui_bitmap_init(&self->bitmap);
	signal_connect(&self->component.signal_draw, self, ondraw);
	signal_connect(&self->component.signal_destroy, self, ondestroy);
	self->alignment = UI_ALIGNMENT_NONE;	
}

void ondestroy(ui_image* self)
{
	ui_bitmap_dispose(&self->bitmap);
}

void ui_image_setbitmapalignment(ui_image* self, UiAlignment alignment)
{
	self->alignment = alignment;
}

void ondraw(ui_image* self, ui_component* sender, ui_graphics* g)
{
	ui_size size;
	ui_size bmpsize;
	int x;
	int y;
	
	size = ui_component_size(&self->component);	
	bmpsize = ui_bitmap_size(&self->bitmap);
	if (checkalignment(self, UI_ALIGNMENT_CENTER_HORIZONTAL)) {
		x = (size.width - bmpsize.width) / 2;
	} else 		
	if (checkalignment(self, UI_ALIGNMENT_RIGHT)) {
		x = size.width - bmpsize.width;		
	} else {		
		x = 0;		
	}
	if (checkalignment(self, UI_ALIGNMENT_CENTER_VERTICAL)) {								
		y = (size.height - bmpsize.height) / 2;
	} else 		
	if (checkalignment(self, UI_ALIGNMENT_BOTTOM)) {
		y = size.height - bmpsize.height;
	} else {		
		y = 0;		
	}	
	ui_drawbitmap(g, &self->bitmap, 
		x, y, bmpsize.width, bmpsize.height, 0, 0);
}

int checkalignment(ui_image* self, UiAlignment alignment)
{
	return (self->alignment & alignment) == alignment;	
}
