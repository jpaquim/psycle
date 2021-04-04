// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uinotebook.h"
#include "uiapp.h"

static void onsize(psy_ui_Notebook*, psy_ui_Component* sender, psy_ui_Size* size);
static void ontabbarchange(psy_ui_Notebook*, psy_ui_Component* sender, int tabindex);

void psy_ui_notebook_init(psy_ui_Notebook* self, psy_ui_Component* parent)
{  
    psy_ui_component_init(psy_ui_notebook_base(self), parent, NULL);
	psy_ui_component_setbackgroundmode(psy_ui_notebook_base(self),
		psy_ui_NOBACKGROUND);
	psy_signal_connect(&psy_ui_notebook_base(self)->signal_size, self, onsize);
	self->pageindex = 0;
	self->split = 0;
}

void psy_ui_notebook_select(psy_ui_Notebook* self, uintptr_t pageindex)
{	
	psy_List* p;
	psy_List* q;
	int c = 0;	
	psy_ui_Size size;
	
	self->pageindex = pageindex;
	if (!self->split) {
		size = psy_ui_component_size(psy_ui_notebook_base(self));
		if (self->component.align == psy_ui_ALIGN_LEFT) {
			size = psy_ui_component_preferredsize(psy_ui_notebook_base(self), &size);
		}
		for (p = q = psy_ui_component_children(psy_ui_notebook_base(self), 0); p != NULL;
			psy_list_next(&p), ++c) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)p->entry;
			if (self->split) {
				psy_ui_component_show(component);
			} else {
				if (c == pageindex) {
					psy_ui_component_show(component);
					psy_ui_component_setposition(component,
						psy_ui_rectangle_make(psy_ui_point_zero(), size));
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
}

uintptr_t psy_ui_notebook_pageindex(psy_ui_Notebook* self)
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
	if (!self->split) {
		psy_List* p;
		psy_List* q;

		int cpx = 0;
		for (p = q = psy_ui_component_children(psy_ui_notebook_base(self), 0); p != NULL; 
				psy_list_next(&p)) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)p->entry;
			psy_ui_component_setposition(component,
				psy_ui_rectangle_make(psy_ui_point_zero(), *size));
		}
		psy_list_free(q);
	} else {
		psy_ui_component_align(&self->component);
	}
}

void ontabbarchange(psy_ui_Notebook* self, psy_ui_Component* sender, int tabindex)
{
	psy_ui_notebook_select(self, tabindex);
}

void psy_ui_notebook_split(psy_ui_Notebook* self, psy_ui_Orientation orientation)
{	
	int c = 0;		
	psy_List* p;
	psy_List* q;

	
	q = psy_ui_component_children(psy_ui_notebook_base(self), 0);			
	for (p = q;  p != NULL; psy_list_next(&p)) {
		psy_ui_Component* component;

		component = (psy_ui_Component*)p->entry;
		psy_ui_component_hide(component);
		component->visible = 1;
	}
	for (p = q, c = 0; p != NULL; psy_list_next(&p), ++c) {
		psy_ui_Component* component;

		component = (psy_ui_Component*)p->entry;
		if (component == &self->splitbar.component) {
			continue;
		}
		//psy_ui_component_hide(component);
		if (c == 0) {
			psy_ui_Size size;
			const psy_ui_TextMetric* tm;
				
			size = psy_ui_component_size(psy_ui_notebook_base(self));
			tm = psy_ui_component_textmetric(psy_ui_notebook_base(self));
			component->preventpreferredsizeatalign = 1;				
			if (orientation == psy_ui_VERTICAL) {
				psy_ui_component_resize(component,
					psy_ui_size_makepx(psy_ui_value_px(&size.width, tm) / 2, 0));
				if (!self->split) {
					psy_ui_splitbar_init(&self->splitbar, psy_ui_notebook_base(self));
					self->split = 1;
					psy_ui_component_setorder(&self->splitbar.component, component);
				}
				psy_ui_component_setalign(component, psy_ui_ALIGN_LEFT);
				psy_ui_component_setalign(&self->splitbar.component, psy_ui_ALIGN_LEFT);
			} else {
				psy_ui_component_resize(component,
					psy_ui_size_makepx(0, psy_ui_value_px(&size.height, tm) / 2));
				if (!self->split) {
					psy_ui_splitbar_init(&self->splitbar, psy_ui_notebook_base(self));
					self->split = 1;
					psy_ui_component_setorder(&self->splitbar.component, component);
				}
				psy_ui_component_setalign(component, psy_ui_ALIGN_TOP);
				psy_ui_component_setalign(&self->splitbar.component, psy_ui_ALIGN_TOP);
			}							
		} else {
			psy_ui_component_setalign(component, psy_ui_ALIGN_CLIENT);
		}
	}
	psy_ui_component_align(&self->component);
	for (p = q; p != NULL; psy_list_next(&p)) {
		psy_ui_Component* component;

		component = (psy_ui_Component*)p->entry;			
		psy_ui_component_show(component);
	}
	psy_list_free(q);
}

int psy_ui_notebook_splitactivated(psy_ui_Notebook* self)
{
	return self->split;
}

void psy_ui_notebook_full(psy_ui_Notebook* self)
{
	if (self->split) {
		psy_ui_component_destroy(&self->splitbar.component);
		self->split = 0;
		psy_ui_notebook_select(self, self->pageindex);		
	}
}

psy_ui_Component* psy_ui_notebook_activepage(psy_ui_Notebook* self)
{	
	return psy_ui_component_at(psy_ui_notebook_base(self), self->pageindex);
}

psy_ui_Component* psy_ui_notebook_page(psy_ui_Notebook* self, uintptr_t pageindex)
{
	return psy_ui_component_at(psy_ui_notebook_base(self), pageindex);
}
