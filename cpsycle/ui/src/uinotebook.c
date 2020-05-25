// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uinotebook.h"

static void onsize(psy_ui_Notebook*, psy_ui_Component* sender, psy_ui_Size* size);
static void align_split(psy_ui_Notebook* self, int x);
static void ontabbarchange(psy_ui_Notebook*, psy_ui_Component* sender, int tabindex);
static void onmousedown(psy_ui_Notebook*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void onmousemove(psy_ui_Notebook*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void onmouseup(psy_ui_Notebook*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void onmouseentersplitbar(psy_ui_Notebook*, psy_ui_Component* sender);
static void onmouseleavesplitbar(psy_ui_Notebook*, psy_ui_Component* sender);

void psy_ui_notebook_init(psy_ui_Notebook* self, psy_ui_Component* parent)
{  
    psy_ui_component_init(psy_ui_notebook_base(self), parent);
	psy_ui_component_setbackgroundmode(psy_ui_notebook_base(self),
		psy_ui_BACKGROUND_NONE);
	psy_signal_connect(&psy_ui_notebook_base(self)->signal_size, self, onsize);
	self->pageindex = 0;
	self->split = 0;
	self->splitx = -1;	
}

void psy_ui_notebook_setpageindex(psy_ui_Notebook* self, int pageindex)
{	
	psy_List* p;
	psy_List* q;
	int c = 0;	
	psy_ui_Size size;
	
	self->pageindex = pageindex;
	size = psy_ui_component_size(psy_ui_notebook_base(self));
	if (self->component.align == psy_ui_ALIGN_LEFT) {
		size = psy_ui_component_preferredsize(psy_ui_notebook_base(self), &size);
	}	
	for (p = q = psy_ui_component_children(psy_ui_notebook_base(self), 0); p != NULL;
			p = p->next, ++c) {		
		psy_ui_Component* component;

		component = (psy_ui_Component*)p->entry;
		if (self->split) {
			psy_ui_component_show(component);
		} else {
			if (c == pageindex) {						
				psy_ui_component_show(component);
				psy_ui_component_setposition(component, 0, 0,
					size.width, size.height);
			} else {		
				psy_ui_component_hide(component);
			}
		}
	}
	psy_list_free(q);		
	if (self->component.align == psy_ui_ALIGN_LEFT) {
        if (psy_ui_component_parent(psy_ui_notebook_base(
                self))) {
            psy_ui_component_align(psy_ui_component_parent(psy_ui_notebook_base(
                self)));
        }        
	}
}

int psy_ui_notebook_pageindex(psy_ui_Notebook* self)
{
	return self->pageindex;
}

void psy_ui_notebook_connectcontroller(psy_ui_Notebook* self, psy_Signal* 
	controllersignal)
{
	psy_signal_connect(controllersignal, self, ontabbarchange);
}

void onsize(psy_ui_Notebook* self, psy_ui_Component* sender, psy_ui_Size* size)
{
	psy_List* p;
	psy_List* q;

	if (self->split) {
		align_split(self, self->splitx);
	} else {
		int cpx = 0;	
		for (p = q = psy_ui_component_children(psy_ui_notebook_base(self), 0); p != NULL; p = p->next) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)p->entry;		
				psy_ui_component_setposition(component,
					0, 0, size->width, size->height);		
		}	
		psy_list_free(q);
	}
}

void ontabbarchange(psy_ui_Notebook* self, psy_ui_Component* sender, int tabindex)
{
	psy_ui_notebook_setpageindex(self, tabindex);
}

void psy_ui_notebook_split(psy_ui_Notebook* self)
{
	if (!self->split) {
		self->split = 1;
		self->splitx = 400;
		psy_ui_component_init(&self->splitbar, psy_ui_notebook_base(self));
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
		psy_ui_notebook_setpageindex(self, 0);
		align_split(self, self->splitx);
	}
}

int psy_ui_notebook_splitactivated(psy_ui_Notebook* self)
{
	return self->split;
}

void psy_ui_notebook_full(psy_ui_Notebook* self)
{
	if (self->split) {
		self->split = 0;
		psy_ui_component_destroy(&self->splitbar);
		psy_ui_notebook_setpageindex(self, self->pageindex);		
	}
}

void onmousedown(psy_ui_Notebook* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	if (self->split) {
		psy_ui_component_capture(sender);
		self->splitx = -1;
	}
}

void onmousemove(psy_ui_Notebook* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	if (self->split && self->splitx == -1) {				
		psy_ui_Size size;
		psy_ui_Rectangle position;		
	
		size = psy_ui_component_size(psy_ui_notebook_base(self));
		position = psy_ui_component_position(sender);
		psy_ui_component_setposition(sender, position.left + ev->x, 0, 4, size.height);		
		align_split(self, position.left + ev->x);
		psy_ui_component_invalidate(psy_ui_notebook_base(self));
		psy_ui_component_update(psy_ui_notebook_base(self));		
	}
}

void onmouseup(psy_ui_Notebook* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	if (self->split) {
		psy_ui_Size size;
		psy_ui_Rectangle position;		
	
		size = psy_ui_component_size(psy_ui_notebook_base(self));
		position = psy_ui_component_position(sender);
		psy_ui_component_move(sender, position.left + ev->x, size.height);
		self->splitx = position.left + ev->x;
		align_split(self, self->splitx);
		psy_ui_component_releasecapture(&self->component);
	}
}

void onmouseentersplitbar(psy_ui_Notebook* self, psy_ui_Component* sender)
{	
	psy_ui_component_setbackgroundcolor(sender, 0x00666666);
	psy_ui_component_invalidate(sender);
}

void onmouseleavesplitbar(psy_ui_Notebook* self, psy_ui_Component* sender)
{			
	psy_ui_component_setbackgroundcolor(sender, 0x00232323);
	psy_ui_component_invalidate(sender);
}

void align_split(psy_ui_Notebook* self, int x) {
	psy_List* p;
	psy_List* q;
	int c = 0;
	psy_ui_Size size;

	size = psy_ui_component_size(psy_ui_notebook_base(self));
	for (p = q = psy_ui_component_children(psy_ui_notebook_base(self), 0); p != NULL; p = p->next) {
		psy_ui_Component* component;
		
		component = (psy_ui_Component*) p->entry;
		if (component == &self->splitbar) {
			psy_ui_component_setposition(&self->splitbar,
				x, 0, 4, size.height);
		} else {
			if (c == 0) {
				psy_ui_component_setposition((psy_ui_Component*)p->entry,
					0, 0, x, size.height);
				++c;
			} else {
				psy_ui_component_setposition((psy_ui_Component*)p->entry,
					x + 4, 0, size.width - x - 4, size.height);
			}			
		}		
	}	
	psy_list_free(q);
}

psy_ui_Component* psy_ui_notebook_activepage(psy_ui_Notebook* self)
{
	psy_ui_Component* rv = 0;
	psy_List* p;
	psy_List* q;

	for (p = q = psy_ui_component_children(psy_ui_notebook_base(self), 0);
			p != NULL; p = p->next) {		
		psy_ui_Component* component;

		component = (psy_ui_Component*)p->entry;
		if (psy_ui_component_visible(component)) {
			rv = component;
			break;
		}
	}
	psy_list_free(q);
	return rv;			
}

psy_ui_Component* psy_ui_notebook_base(psy_ui_Notebook* self)
{
	return &self->component;
}
