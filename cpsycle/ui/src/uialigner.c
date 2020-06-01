// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uialigner.h"

static uintptr_t psy_ui_aligner_numclients(psy_ui_Aligner*);
static void psy_ui_align_alignclients(psy_ui_Aligner*, psy_List* children,
	psy_ui_Point cp_topleft, psy_ui_Point cp_bottomright);

void psy_ui_aligner_init(psy_ui_Aligner* self, psy_ui_Component* component)
{
	self->component = component;
}

void psy_ui_aligner_align(psy_ui_Aligner* self)
{	
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	psy_ui_Point cp_topleft = { 0, 0 };
	psy_ui_Point cp_bottomright = { 0, 0 };	
	int cpymax = 0;
	psy_List* p;
	psy_List* q;
	psy_List* wrap = 0;	
	psy_ui_Component* client = 0;
	
	size = psy_ui_component_size(self->component);
	tm = psy_ui_component_textmetric(self->component);
	cp_bottomright.x = psy_ui_value_px(&size.width, &tm);
	cp_bottomright.y = psy_ui_value_px(&size.height, &tm);
	for (p = q = psy_ui_component_children(self->component, 0); p != NULL;
			p = p->next) {
		psy_ui_Component* component;
			
		component = (psy_ui_Component*)p->entry;		
		if (component->visible) {
			psy_ui_Size componentsize;
			psy_ui_Size limit;

			limit.width = psy_ui_value_makepx(cp_bottomright.x - cp_topleft.x);
			limit.height = psy_ui_value_makepx(cp_bottomright.y - cp_topleft.y);
			if (component->debugflag == 9000) {
				self = self;
			}
			componentsize = psy_ui_component_preferredsize(component, &limit);
			if (component->align == psy_ui_ALIGN_CLIENT) {
				client = component;
			} else
			if (component->align == psy_ui_ALIGN_FILL) {
				psy_ui_component_setposition(component,
					psy_ui_value_px(&component->margin.left, &tm),
					psy_ui_value_px(&component->margin.top, &tm),				
					psy_ui_value_makepx(
						psy_ui_value_px(&size.width, &tm) - psy_ui_value_px(&component->margin.left, &tm)
						- psy_ui_value_px(&component->margin.right, &tm)),
					psy_ui_value_makepx(psy_ui_value_px(&size.height, &tm) -
						psy_ui_margin_height_px(&component->margin, &tm)));
			} else
			if (component->align == psy_ui_ALIGN_TOP) {
				cp_topleft.y += psy_ui_value_px(&component->margin.top, &tm);
				psy_ui_component_setposition(component, 
					cp_topleft.x + psy_ui_value_px(&component->margin.left, &tm),
					cp_topleft.y,
					psy_ui_value_makepx(cp_bottomright.x - cp_topleft.x -
						psy_ui_margin_width_px(&component->margin, &tm)),
					componentsize.height);
				cp_topleft.y += psy_ui_value_px(&component->margin.bottom, &tm);
				cp_topleft.y += psy_ui_value_px(&componentsize.height, &tm);
			} else
			if (component->align == psy_ui_ALIGN_BOTTOM) {				
				cp_bottomright.y -=
					psy_ui_value_px(&component->margin.bottom, &tm);
				psy_ui_component_setposition(component, 
					cp_topleft.x + psy_ui_value_px(&component->margin.left, &tm), 
					cp_bottomright.y - psy_ui_value_px(&componentsize.height, &tm),
					psy_ui_value_makepx(cp_bottomright.x - cp_topleft.x -
						psy_ui_margin_width_px(&component->margin, &tm)),
					componentsize.height);
				cp_bottomright.y -= psy_ui_value_px(&component->margin.top, &tm);
				cp_bottomright.y -= psy_ui_value_px(&componentsize.height, &tm);
			} else
			if (component->align == psy_ui_ALIGN_RIGHT) {
				int requiredcomponentwidth;

				requiredcomponentwidth =
					psy_ui_value_px(&componentsize.width, &tm) +
					psy_ui_margin_width_px(&component->margin, &tm);
				cp_bottomright.x -= requiredcomponentwidth;
				psy_ui_component_setposition(component,
					cp_bottomright.x + psy_ui_value_px(&component->margin.left, &tm),
					cp_topleft.y +
						psy_ui_value_px(&component->margin.top, &tm),
					componentsize.width,
					psy_ui_value_makepx(cp_bottomright.y - cp_topleft.y -
						psy_ui_margin_height_px(&component->margin, &tm)));
			} else
			if (component->align == psy_ui_ALIGN_LEFT) {				
				if ((self->component->alignexpandmode & psy_ui_HORIZONTALEXPAND)
						== psy_ui_HORIZONTALEXPAND) {
				} else {
					int requiredcomponentwidth;

					requiredcomponentwidth = psy_ui_value_px(&componentsize.width, &tm) +
						psy_ui_margin_width_px(&component->margin, &tm);
					if (cp_topleft.x + requiredcomponentwidth > psy_ui_value_px(&size.width, &tm)) {
						psy_List* w;						
						cp_topleft.x = 0;
						for (w = wrap; w != 0; w = w->next) {
							psy_ui_Component* c;
							c = (psy_ui_Component*)w->entry;
							psy_ui_component_resize(c,
								psy_ui_component_size(c).width,
								psy_ui_value_makepx(cpymax - cp_topleft.y -
									psy_ui_margin_height_px(&component->margin, &tm)));
						}
						cp_topleft.y = cpymax;
						psy_list_free(wrap);						
						wrap = 0;
					}					
					psy_list_append(&wrap, component);					
				}
				cp_topleft.x += psy_ui_value_px(&component->margin.left, &tm);
				psy_ui_component_setposition(component,
					cp_topleft.x,
					cp_topleft.y + psy_ui_value_px(&component->margin.top, &tm),
					componentsize.width,
					psy_ui_value_makepx(component->justify == psy_ui_JUSTIFY_EXPAND
					? cp_bottomright.y - cp_topleft.y - 
						psy_ui_margin_height_px(&component->margin, &tm)
					: psy_ui_value_px(&componentsize.height, &tm)));
				cp_topleft.x += psy_ui_value_px(&component->margin.right, &tm);
				cp_topleft.x += psy_ui_value_px(&componentsize.width, &tm);
				if (cpymax < cp_topleft.y + psy_ui_value_px(&componentsize.height, &tm) +
						psy_ui_margin_height_px(&component->margin, &tm)) {
					cpymax = cp_topleft.y + psy_ui_value_px(&componentsize.height, &tm) +
						psy_ui_margin_height_px(&component->margin, &tm);
				}
			}				
		}
	}
	psy_ui_align_alignclients(self, q, cp_topleft, cp_bottomright);
	psy_list_free(q);
	psy_list_free(wrap);	
}

void psy_ui_align_alignclients(psy_ui_Aligner* self, psy_List* children,
	psy_ui_Point cp_topleft, psy_ui_Point cp_bottomright)
{
	psy_List* p;
	uintptr_t curr;
	double height;
	psy_ui_TextMetric tm;	
	uintptr_t numclients;

	numclients = psy_ui_aligner_numclients(self);
	if (numclients != 0) {
		tm = psy_ui_component_textmetric(self->component);
		height = (cp_bottomright.y - cp_topleft.y) / numclients;
		for (curr = 0, p = children; p != NULL; p = p->next) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)p->entry;
			if (component->align == psy_ui_ALIGN_CLIENT) {
				cp_topleft.y += psy_ui_value_px(&component->margin.top, &tm);
				psy_ui_component_setposition(component,
					cp_topleft.x + psy_ui_value_px(&component->margin.left, &tm),
					cp_topleft.y,
					psy_ui_value_makepx(cp_bottomright.x - cp_topleft.x -
					psy_ui_margin_width_px(&component->margin, &tm)),
					psy_ui_value_makepx((int)height));
				cp_topleft.y += psy_ui_value_px(&component->margin.bottom, &tm);
				cp_topleft.y += (int)height;
				++curr;
			}
		}
	}
}

uintptr_t psy_ui_aligner_numclients(psy_ui_Aligner* self)
{
	uintptr_t rv = 0;
	psy_List* p;
		
	for (p = psy_ui_component_children(self->component, 0); p != NULL;
		p = p->next) {
		psy_ui_Component* component;

		component = (psy_ui_Component*)p->entry;
		if (component->align == psy_ui_ALIGN_CLIENT) {
			++rv;
		}
	}
	psy_list_free(p);
	return rv;
}

void psy_ui_aligner_preferredsize(psy_ui_Aligner* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{			
	if (rv) {
		psy_ui_Size size;
		psy_ui_TextMetric tm;
		psy_ui_Component* client = 0;

		size = psy_ui_component_size(self->component);
		tm = psy_ui_component_textmetric(self->component);
		if (self->component->alignchildren && !self->component->preventpreferredsize) {
			psy_List* p;
			psy_List* q;
			psy_ui_Point cp = { 0, 0 };
			psy_ui_Size maxsize = { 0, 0 };
			psy_ui_Point cp_topleft = { 0, 0 };
			psy_ui_Point cp_bottomright = { 0, 0 };
			
			
			size.width = (self->component->alignexpandmode &
				psy_ui_HORIZONTALEXPAND) == psy_ui_HORIZONTALEXPAND
				? psy_ui_value_makepx(0)
				: limit->width;
			for (p = q = psy_ui_component_children(self->component, 0); p != NULL;
					p = p->next) {
				psy_ui_Component* component;
					
				component = (psy_ui_Component*)p->entry;		
				if (component->visible) {
					psy_ui_Size componentsize;
					psy_ui_Size limit;

					limit.width = psy_ui_value_makepx(psy_ui_value_px(&size.width, &tm) - cp_topleft.x -
						cp_bottomright.x);
					limit.height = size.height;
					componentsize = psy_ui_component_preferredsize(component, &limit);
					if (component->align == psy_ui_ALIGN_CLIENT) {
						if (psy_ui_value_px(&maxsize.height, &tm) < cp.y + psy_ui_value_px(&componentsize.height, &tm) +
							psy_ui_margin_height_px(&component->margin, &tm)) {
							maxsize.height = psy_ui_value_makepx(cp.y + psy_ui_value_px(&componentsize.height, &tm) +
								psy_ui_margin_height_px(&component->margin, &tm));
						}
						if (psy_ui_value_px(&maxsize.width, &tm) < psy_ui_value_px(&componentsize.width, &tm) +
							psy_ui_margin_width_px(&component->margin, &tm)) {
							maxsize.width = psy_ui_value_makepx(psy_ui_value_px(&componentsize.width, &tm) +
								psy_ui_margin_width_px(&component->margin, &tm));
						}
					} else
					if (component->align == psy_ui_ALIGN_TOP ||
							component->align == psy_ui_ALIGN_BOTTOM) {
						cp.y += psy_ui_value_px(&componentsize.height, &tm) +
							psy_ui_margin_height_px(&component->margin, &tm);
						if (psy_ui_value_px(&maxsize.height, &tm) < cp.y) {
							maxsize.height = psy_ui_value_makepx(cp.y);
						}
						if (psy_ui_value_px(&maxsize.width, &tm) < psy_ui_value_px(&componentsize.width, &tm) +
								psy_ui_margin_width_px(&component->margin, &tm)) {
							maxsize.width = psy_ui_value_makepx(
								psy_ui_value_px(&componentsize.width, &tm) +
								psy_ui_margin_width_px(&component->margin, &tm));
						}
					} else
					if (component->align == psy_ui_ALIGN_RIGHT) {
						cp.x += psy_ui_value_px(&componentsize.width, &tm) +
							psy_ui_margin_width_px(&component->margin, &tm);
						cp_bottomright.x += psy_ui_value_px(&componentsize.width, &tm) +
							psy_ui_margin_width_px(&component->margin, &tm);
						if (psy_ui_value_px(&maxsize.width, &tm) < cp.x) {
							maxsize.width = psy_ui_value_makepx(cp.x);
						}
						if (psy_ui_value_px(&maxsize.height, &tm) < cp.y + psy_ui_value_px(&componentsize.height, &tm) +
								psy_ui_margin_height_px(&component->margin, &tm)) {
							maxsize.height = psy_ui_value_makepx(
								cp.y + psy_ui_value_px(&componentsize.height, &tm) +
								psy_ui_margin_height_px(&component->margin, &tm));
						}												
					} else
					if (component->align == psy_ui_ALIGN_LEFT) {
						if (psy_ui_value_px(&size.width, &tm) != 0) {
							int requiredcomponentwidth;

							requiredcomponentwidth = psy_ui_value_px(&componentsize.width, &tm) +
								psy_ui_margin_width_px(&component->margin, &tm);
							if (cp.x + requiredcomponentwidth > psy_ui_value_px(&size.width, &tm)) {
								cp.y = psy_ui_value_px(&maxsize.height, &tm);
								cp.x = 0;
								cp_topleft.x = 0;
							}						
						}						
						cp.x += psy_ui_value_px(&componentsize.width, &tm) +
							psy_ui_margin_width_px(&component->margin, &tm);
						cp_topleft.x += psy_ui_value_px(&componentsize.width, &tm) +
							psy_ui_margin_width_px(&component->margin, &tm);
						if (psy_ui_value_px(&maxsize.width, &tm) < cp.x) {
							maxsize.width = psy_ui_value_makepx(cp.x);
						}
						if (psy_ui_value_px(&maxsize.height, &tm) < cp.y + psy_ui_value_px(&componentsize.height, &tm) +
								psy_ui_margin_height_px(&component->margin, &tm)) {
							maxsize.height = psy_ui_value_makepx(cp.y + psy_ui_value_px(&componentsize.height, &tm) +
								psy_ui_margin_height_px(&component->margin, &tm));
						}
					}				
				}
			}
			psy_list_free(q);
			*rv = maxsize;
			rv->width = psy_ui_value_makepx(psy_ui_value_px(&rv->width, &tm) +
				psy_ui_margin_width_px(&self->component->spacing, &tm));
			rv->height = psy_ui_value_makepx(psy_ui_value_px(&rv->height, &tm) +
				psy_ui_margin_height_px(&self->component->spacing, &tm));
		} else {
			*rv = size;
			rv->width = psy_ui_value_makepx(psy_ui_value_px(&rv->width, &tm) +
				psy_ui_margin_width_px(&self->component->spacing, &tm));
			rv->height = psy_ui_value_makepx(psy_ui_value_px(&rv->height, &tm) +
				psy_ui_margin_height_px(&self->component->spacing, &tm));
		}
	}	
}
