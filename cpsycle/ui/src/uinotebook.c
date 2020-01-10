// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uinotebook.h"
#include "uiwincomponent.h"

static void onsize(ui_notebook*, psy_ui_Component* sender, ui_size* size);
static void align_split(ui_notebook* self, int x);
static void ontabbarchange(ui_notebook*, psy_ui_Component* sender, int tabindex);
static void onmousedown(ui_notebook*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void onmousemove(ui_notebook*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void onmouseup(ui_notebook*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void onmouseentersplitbar(ui_notebook*, psy_ui_Component* sender);
static void onmouseleavesplitbar(ui_notebook*, psy_ui_Component* sender);

void ui_notebook_init(ui_notebook* self, psy_ui_Component* parent)
{  
    ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	self->component.defaultpropagation = 1;
	psy_signal_connect(&self->component.signal_size, self, onsize);
	self->pageindex = 0;
	self->split = 0;
	self->splitx = -1;
	self->splitbar.platform = 0;
}

void ui_notebook_setpageindex(ui_notebook* self, int pageindex)
{	
	psy_List* p;
	psy_List* q;
	int c = 0;	
	ui_size size;
	
	self->pageindex = pageindex;
	size = ui_component_size(&self->component);
	if (self->component.align == UI_ALIGN_LEFT) {
		size = ui_component_preferredsize(&self->component, &size);
	}	
	for (p = q = ui_component_children(&self->component, 0); p != NULL;
			p = p->next, ++c) {		
		psy_ui_Component* component;

		component = (psy_ui_Component*)p->entry;
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
	psy_list_free(q);		
	if (self->component.align == UI_ALIGN_LEFT) {
		ui_component_align(ui_component_parent(&self->component));
	}
}

int ui_notebook_pageindex(ui_notebook* self)
{
	return self->pageindex;
}

void ui_notebook_connectcontroller(ui_notebook* self, psy_Signal* 
	controllersignal)
{
	psy_signal_connect(controllersignal, self, ontabbarchange);
}

void onsize(ui_notebook* self, psy_ui_Component* sender, ui_size* size)
{
	psy_List* p;
	psy_List* q;

	if (self->split) {
		align_split(self, self->splitx);
	} else {
		int cpx = 0;	
		for (p = q = ui_component_children(&self->component, 0); p != 0; p = p->next) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)p->entry;		
				ui_component_setposition(component,
					0, 0, size->width, size->height);		
		}	
		psy_list_free(q);
	}
}

void ontabbarchange(ui_notebook* self, psy_ui_Component* sender, int tabindex)
{
	ui_notebook_setpageindex(self, tabindex);
}

void ui_notebook_split(ui_notebook* self)
{
	if (self->splitbar.platform == 0) {
		self->split = 1;
		self->splitx = 400;
		ui_component_init(&self->splitbar, &self->component);
		psy_signal_connect(&self->splitbar.signal_mouseenter, self,
			onmouseentersplitbar);
		psy_signal_connect(&self->splitbar.signal_mouseleave, self,
			onmouseleavesplitbar);
		psy_signal_connect(&self->splitbar.signal_mousedown, self,
			onmousedown);
		psy_signal_connect(&self->splitbar.signal_mousemove, self,
			onmousemove);
		psy_signal_connect(&self->splitbar.signal_mouseup, self,
			onmouseup);
		ui_notebook_setpageindex(self, 0);
		align_split(self, self->splitx);
	}
}

void ui_notebook_full(ui_notebook* self)
{
	if (self->splitbar.platform != 0) {
		self->split = 0;
		ui_component_destroy(&self->splitbar);
		ui_notebook_setpageindex(self, self->pageindex);		
	}
}


void onmousedown(ui_notebook* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	if (self->split) {
		ui_component_capture(sender);
		self->splitx = -1;
	}
}

void onmousemove(ui_notebook* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	if (self->split && self->splitx == -1) {				
		ui_size size;
		ui_rectangle position;		
	
		size = ui_component_size(&self->component);
		position = ui_component_position(sender);
		ui_component_setposition(sender, position.left + ev->x, 0, 4, size.height);		
		align_split(self, position.left + ev->x);
		ui_component_invalidate(&self->component);
		ui_component_update(&self->component);		
	}
}

void onmouseup(ui_notebook* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	if (self->split) {
		ui_size size;
		ui_rectangle position;		
	
		size = ui_component_size(&self->component);
		position = ui_component_position(sender);
		ui_component_move(sender, position.left + ev->x, size.height);
		self->splitx = position.left + ev->x;
		align_split(self, self->splitx);
		ui_component_releasecapture();
	}
}

void onmouseentersplitbar(ui_notebook* self, psy_ui_Component* sender)
{	
	ui_component_setbackgroundcolor(sender, 0x00666666);
	ui_component_invalidate(sender);
}

void onmouseleavesplitbar(ui_notebook* self, psy_ui_Component* sender)
{			
	ui_component_setbackgroundcolor(sender, 0x00232323);
	ui_component_invalidate(sender);
}

void align_split(ui_notebook* self, int x) {
	psy_List* p;
	psy_List* q;
	int c = 0;
	ui_size size;

	size = ui_component_size(&self->component);
	for (p = q = ui_component_children(&self->component, 0); p != 0; p = p->next) {
		psy_ui_Component* component;
		
		component = (psy_ui_Component*)p->entry;		
		if (ui_win_component_hwnd(component) == 
				ui_win_component_hwnd(&self->splitbar)) {
			ui_component_setposition(&self->splitbar,
				x, 0, 4, size.height);
		} else {
			if (c == 0) {
				ui_component_setposition((psy_ui_Component*)p->entry,
					0, 0, x, size.height);
				++c;
			} else {
				ui_component_setposition((psy_ui_Component*)p->entry,
					x + 4, 0, size.width - x - 4, size.height);
			}			
		}		
	}	
	psy_list_free(q);
}

psy_ui_Component* ui_notebook_activepage(ui_notebook* self)
{
	psy_ui_Component* rv = 0;
	psy_List* p;
	psy_List* q;

	for (p = q = ui_component_children(&self->component, 0);
			p != 0; p = p->next) {		
		psy_ui_Component* component;

		component = (psy_ui_Component*)p->entry;
		if (ui_component_visible(component)) {
			rv = component;
			break;
		}
	}
	psy_list_free(q);
	return rv;			
}
