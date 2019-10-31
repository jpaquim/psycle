// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiprogressbar.h"
#include <string.h>

static void ondestroy(ui_progressbar*, ui_component* sender);
static void ondraw(ui_progressbar*, ui_component* sender, ui_graphics*);
static void onpreferredsize(ui_progressbar*, ui_component* sender, ui_size* limit,
	int* width, int* height);

void ui_progressbar_init(ui_progressbar* self, ui_component* parent)
{	
	self->text = _strdup("");
	self->progress = 0.f;
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, ondraw);
	signal_connect(&self->component.signal_destroy, self, ondestroy);
	signal_connect(&self->component.signal_preferredsize, self,
		onpreferredsize);
}

void ondestroy(ui_progressbar* self, ui_component* sender)
{	
	free(self->text);	
}

void ondraw(ui_progressbar* self, ui_component* sender, ui_graphics* g)
{
	ui_size size;
	ui_rectangle r;
	int height;
		
	height = 10;
	size = ui_component_size(&self->component);	
	ui_setrectangle(&r, 0, 
		(size.height - height) / 2, 
		(int)(self->progress * size.width),
		height);
	ui_drawsolidrectangle(g, r, 0x00D1C5B6);
}

void onpreferredsize(ui_progressbar* self, ui_component* sender, ui_size* limit,
	int* width, int* height)
{		
	*width = 200;
	*height = 20;	
}

void ui_progressbar_setprogress(ui_progressbar* self, float progress)
{
	self->progress = progress;
	ui_invalidate(&self->component);
	UpdateWindow((HWND)self->component.hwnd);
}

void ui_progressbar_tick(ui_progressbar* self)
{
	self->progress += 0.01f;
	ui_invalidate(&self->component);
	UpdateWindow((HWND)self->component.hwnd);
}