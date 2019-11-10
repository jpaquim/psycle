// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uinotebook.h"

static void onsize(ui_notebook*, ui_component* sender, ui_size* size);
static void align_split(ui_notebook* self, int x);
static void ontabbarchange(ui_notebook*, ui_component* sender, int tabindex);
static void onmousedown(ui_notebook*, ui_component* sender, int x, int y, int button);
static void onmousemove(ui_notebook*, ui_component* sender, int x, int y, int button);
static void onmouseup(ui_notebook*, ui_component* sender, int x, int y, int button);
static void onmouseentersplitbar(ui_notebook*, ui_component* sender);
static void onmouseleavesplitbar(ui_notebook*, ui_component* sender);

void ui_notebook_init(ui_notebook* self, ui_component* parent)
{  
    ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	self->component.defaultpropagation = 1;
	signal_connect(&self->component.signal_size, self, onsize);
	self->pageindex = 0;
	self->split = 0;
	self->splitx = -1;
	self->splitbar.hwnd = 0;	
}

void ui_notebook_setpage(ui_notebook* self, int pageindex)
{	
	List* p;
	List* q;
	int c = 0;	
	ui_size size;
	
	self->pageindex = pageindex;
	size = ui_component_size(&self->component);
	if (self->component.align == UI_ALIGN_LEFT) {
		size = ui_component_preferredsize(&self->component, &size);
	}	
	for (p = q = ui_component_children(&self->component, 0); p != NULL;
			p = p->next, ++c) {		
		ui_component* component;

		component = (ui_component*)p->entry;
		if (self->split) {
			ui_component_show(component);			
		} else {
			if (c == pageindex) {						
				ui_component_show(component);
				ui_component_setposition(component, 0, 0,
					size.width, size.height);
			} else {		
				ui_component_hide(component);
			}
		}
	}
	list_free(q);		
	if (self->component.align == UI_ALIGN_LEFT) {
		ui_component_align(ui_component_parent(&self->component));
	}
}

int ui_notebook_page(ui_notebook* self)
{
	return self->pageindex;
}

void ui_notebook_connectcontroller(ui_notebook* self, Signal* controllersignal)
{
	signal_connect(controllersignal, self, ontabbarchange);
}

void onsize(ui_notebook* self, ui_component* sender, ui_size* size)
{
	List* p;
	List* q;

	if (self->split) {
		align_split(self, self->splitx);
	} else {
		int cpx = 0;	
		for (p = q = ui_component_children(&self->component, 0); p != 0; p = p->next) {
			ui_component* component;

			component = (ui_component*)p->entry;		
				ui_component_setposition(component,
					0, 0, size->width, size->height);		
		}	
		list_free(q);
	}
}

void ontabbarchange(ui_notebook* self, ui_component* sender, int tabindex)
{
	ui_notebook_setpage(self, tabindex);
}

void ui_notebook_split(ui_notebook* self)
{
	if (self->splitbar.hwnd == 0) {
		self->split = 1;
		self->splitx = 200;
		ui_component_init(&self->splitbar, &self->component);
		signal_connect(&self->splitbar.signal_mouseenter, self, onmouseentersplitbar);
		signal_connect(&self->splitbar.signal_mouseleave, self, onmouseleavesplitbar);
		signal_connect(&self->splitbar.signal_mousedown, self, onmousedown);		
		signal_connect(&self->splitbar.signal_mousemove, self, onmousemove);
		signal_connect(&self->splitbar.signal_mouseup, self, onmouseup);
		ui_notebook_setpage(self, 0);
		align_split(self, self->splitx);
	}
}

void ui_notebook_full(ui_notebook* self)
{
	if (self->splitbar.hwnd != 0) {
		self->split = 0;
		ui_component_destroy(&self->splitbar);
		ui_notebook_setpage(self, self->pageindex);
		self->splitbar.hwnd = 0;
	}
}


void onmousedown(ui_notebook* self, ui_component* sender, int x, int y, int button)
{	
	if (self->split) {
		ui_component_capture(sender);
		self->splitx = -1;
	}
}

void onmousemove(ui_notebook* self, ui_component* sender, int x, int y, int button)
{
	if (self->split && self->splitx == -1) {				
		ui_size size;
		ui_rectangle position;		
	
		size = ui_component_size(&self->component);
		position = ui_component_position(sender);
		ui_component_move(sender, position.left + x, size.height);		
		align_split(self, position.left + x);
		ui_invalidate(&self->component);
		ui_component_update(&self->component);		
	}
}

void onmouseup(ui_notebook* self, ui_component* sender, int x, int y, int button)
{	
	if (self->split) {
		ui_size size;
		ui_rectangle position;		
	
		size = ui_component_size(&self->component);
		position = ui_component_position(sender);
		ui_component_move(sender, position.left + x, size.height);
		self->splitx = position.left + x;
		align_split(self, self->splitx);
		ui_component_releasecapture();
	}
}

void onmouseentersplitbar(ui_notebook* self, ui_component* sender)
{	
	ui_component_setbackgroundcolor(sender, 0x00666666);
	ui_invalidate(sender);
}

void onmouseleavesplitbar(ui_notebook* self, ui_component* sender)
{			
	ui_component_setbackgroundcolor(sender, 0x00232323);
	ui_invalidate(sender);
}

void align_split(ui_notebook* self, int x) {
	List* p;
	List* q;
	int c = 0;
	ui_size size;

	size = ui_component_size(&self->component);
	for (p = q = ui_component_children(&self->component, 0); p != 0; p = p->next) {
		ui_component* component;
		
		component = (ui_component*)p->entry;		
		if (component->hwnd == self->splitbar.hwnd) {
			ui_component_setposition(&self->splitbar,
				x, 0, 4, size.height);
		} else {
			if (c == 0) {
				ui_component_setposition((ui_component*)p->entry,
					0, 0, x, size.height);
				++c;
			} else {
				ui_component_setposition((ui_component*)p->entry,
					x + 4, 0, size.width - x - 4, size.height);
			}			
		}		
	}	
	list_free(q);
}
