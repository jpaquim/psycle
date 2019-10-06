// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uinotebook.h"

void onsize(ui_notebook*, ui_component* sender, int width, int height);
void ontabbarchange(ui_notebook*, ui_component* sender, int tabindex);

void ui_notebook_init(ui_notebook* self, ui_component* parent)
{  
    ui_component_init(&self->component, parent);
	self->component.defaultpropagation = 1;
	signal_connect(&self->component.signal_size, self, onsize);
	self->pageindex = 0;
}

void ui_notebook_setpage(ui_notebook* self, int pageindex)
{	
	List* p;
	int c = 0;		
	ui_size size;

	self->pageindex = pageindex;
	size = ui_component_size(&self->component);
	for (p = ui_component_children(&self->component, 0); p != NULL;
			p = p->next, ++c) {		
		ui_component* component;

		component = (ui_component*)p->entry;
		if (c == pageindex) {		
			int visible;
			
			visible = ui_component_visible(component);
			ui_component_show(component);
			ui_component_resize(component, size.width, size.height);
			//if (visible) {				
			//	SendMessage(component->hwnd, WM_SHOWWINDOW, TRUE, SW_PARENTOPENING);
			//}
		} else {
			ui_component_hide(component);
		}
	}
	list_free(p);	
}

int ui_notebook_page(ui_notebook* self)
{
	return self->pageindex;
}

void ui_notebook_connectcontroller(ui_notebook* self, Signal* controllersignal)
{
	signal_connect(controllersignal, self, ontabbarchange);
}

void onsize(ui_notebook* self, ui_component* sender, int width, int height)
{
	List* p;

	for (p = ui_component_children(&self->component, 0); p != 0; p = p->next) {			
		ui_component_resize((ui_component*)p->entry, width, height);
	}
	list_free(p);
}

void ontabbarchange(ui_notebook* self, ui_component* sender, int tabindex)
{
	ui_notebook_setpage(self, tabindex);
}
