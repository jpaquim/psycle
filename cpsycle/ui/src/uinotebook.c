/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uinotebook.h"
/* local */
#include "uiapp.h"

/* prototypes */
static void psy_ui_notebook_ontabbarchange(psy_ui_Notebook*,
	psy_ui_Component* sender, uintptr_t tabindex);
static void psy_ui_notebook_onalign(psy_ui_Notebook*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_Notebook* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super_vtable = *(self->component.vtable);		
		vtable.onalign =
			(psy_ui_fp_component_event)
			psy_ui_notebook_onalign;
		vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(&self->component, &vtable);
}

/* implementation */
void psy_ui_notebook_init(psy_ui_Notebook* self, psy_ui_Component* parent)
{  
    psy_ui_component_init(psy_ui_notebook_base(self), parent, NULL);
	vtable_init(self);
	psy_ui_component_setbackgroundcolour(psy_ui_notebook_base(self),
		psy_ui_colour_transparent());
	self->pageindex = 0;
	self->split = 0;
	self->preventalign = FALSE;
}

void psy_ui_notebook_select(psy_ui_Notebook* self, uintptr_t pageindex)
{	
	psy_List* p;
	psy_List* q;
	int c = 0;	
	psy_ui_Size size;
	
	self->pageindex = pageindex;
	if (!self->split) {
		size = psy_ui_component_scrollsize(psy_ui_notebook_base(self));
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
					psy_ui_Size oldsize;
					bool resize;					
					oldsize = psy_ui_component_scrollsize(component);
					resize = (psy_ui_value_px(&oldsize.width, psy_ui_component_textmetric(component), NULL) !=
						psy_ui_value_px(&size.width, psy_ui_component_textmetric(component), NULL)) ||
						(psy_ui_value_px(&oldsize.height, psy_ui_component_textmetric(component), NULL) !=
						psy_ui_value_px(&size.height, psy_ui_component_textmetric(component), NULL));
					if (!self->preventalign) {
						psy_ui_component_setposition(component,
							psy_ui_rectangle_make(psy_ui_point_zero(), size));
						if (!resize) {
							psy_ui_component_align(component);
						}
					}
					psy_ui_component_show(component);
					psy_ui_component_invalidate(component);
				} 
			}
		}
		for (p = q, c = 0; p != NULL; psy_list_next(&p), ++c) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)p->entry;
			if (!self->split && c != pageindex) {								
				psy_ui_component_hide(component);				
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
	psy_signal_connect(controllersignal, self, psy_ui_notebook_ontabbarchange);
}

void psy_ui_notebook_onalign(psy_ui_Notebook* self)
{
	if (!self->split) {
		psy_List* p;
		psy_List* q;

		int cpx = 0;
		for (p = q = psy_ui_component_children(psy_ui_notebook_base(self), 0); p != NULL; 
				psy_list_next(&p)) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)p->entry;
			if (psy_ui_component_visible(component)) {
				psy_ui_Size size;

				size = psy_ui_component_scrollsize(&self->component);				
				psy_ui_component_setposition(component,
					psy_ui_rectangle_make(psy_ui_point_zero(), size));				
			}
		}
		psy_list_free(q);
	}
}

void psy_ui_notebook_ontabbarchange(psy_ui_Notebook* self,
	psy_ui_Component* sender, uintptr_t tabindex)
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
		psy_ui_component_show(component);		
	}
	for (p = q, c = 0; p != NULL; psy_list_next(&p), ++c) {
		psy_ui_Component* component;

		component = (psy_ui_Component*)p->entry;
		if (component == &self->splitbar.component) {
			continue;
		}	
		if (c == 0) {
			psy_ui_Size size;
			const psy_ui_TextMetric* tm;
				
			size = psy_ui_component_scrollsize(psy_ui_notebook_base(self));
			tm = psy_ui_component_textmetric(psy_ui_notebook_base(self));			
			if (orientation == psy_ui_VERTICAL) {
				psy_ui_component_set_preferred_size(component,
					psy_ui_size_make_px(psy_ui_value_px(&size.width, tm, NULL) / 2, 0));
				if (!self->split) {
					psy_ui_splitter_init(&self->splitbar, psy_ui_notebook_base(self));
					self->split = 1;
					psy_ui_component_setorder(&self->splitbar.component, component);
				}
				psy_ui_component_set_align(component, psy_ui_ALIGN_LEFT);
				psy_ui_component_set_align(&self->splitbar.component, psy_ui_ALIGN_LEFT);
			} else {
				psy_ui_component_set_preferred_size(component,
					psy_ui_size_make_px(0, psy_ui_value_px(&size.height, tm, NULL) / 2));
				if (!self->split) {
					psy_ui_splitter_init(&self->splitbar,
						psy_ui_notebook_base(self));
					self->split = 1;
					psy_ui_component_setorder(&self->splitbar.component, component);
				}
				psy_ui_component_set_align(component, psy_ui_ALIGN_TOP);
				psy_ui_component_set_align(&self->splitbar.component, psy_ui_ALIGN_TOP);
			}							
		} else {
			psy_ui_component_set_align(component, psy_ui_ALIGN_CLIENT);
		}
	}
	psy_ui_component_align(&self->component);	
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
