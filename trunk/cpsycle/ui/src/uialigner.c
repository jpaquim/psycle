// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uialigner.h"
// std
#include <math.h>
#include <stdio.h>

// prototypes
static void psy_ui_aligner_adjustminmaxsize(psy_ui_Aligner*,
	psy_ui_Component*, const psy_ui_TextMetric*,
	psy_ui_Size*);
static uintptr_t psy_ui_aligner_numclients(psy_ui_Aligner*);
static void psy_ui_align_alignclients(psy_ui_Aligner*, psy_List* children,
	psy_ui_RealPoint cp_topleft, psy_ui_RealPoint cp_bottomright);
static void psy_ui_aligner_adjustborder(psy_ui_Aligner*,
	psy_ui_RealPoint* cp_topleft, psy_ui_RealPoint* cp_bottomright);
static void psy_ui_aligner_adjustspacing(psy_ui_Aligner*,
	psy_ui_RealPoint* cp_topleft, psy_ui_RealPoint* cp_bottomright);
static void psy_ui_aligner_resizewrapline(psy_ui_Aligner*, psy_List* wrap,
	double cpy, double cpymax);
// implementation
void psy_ui_aligner_init(psy_ui_Aligner* self, psy_ui_Component* component)
{
	self->component = component;
}

void psy_ui_aligner_align(psy_ui_Aligner* self)
{	
	psy_ui_Size size;
	const psy_ui_TextMetric* tm;
	psy_ui_RealPoint cp_topleft;
	psy_ui_RealPoint cp_bottomright;
	double cpymax = 0;
	psy_List* p;
	psy_List* q;
	psy_List* wrap = 0;		
	
	size = psy_ui_component_size(self->component);
	tm = psy_ui_component_textmetric(self->component);
	psy_ui_realpoint_init(&cp_topleft);
	psy_ui_realpoint_init_all(&cp_bottomright,
		floor(psy_ui_value_px(&size.width, tm)),
		floor(psy_ui_value_px(&size.height, tm)));
	psy_ui_aligner_adjustborder(self, &cp_topleft, &cp_bottomright);
	psy_ui_aligner_adjustspacing(self, &cp_topleft, &cp_bottomright);
	
	for (p = q = psy_ui_component_children(self->component, 0); p != NULL;
			psy_list_next(&p)) {
		psy_ui_Component* component;
			
		component = (psy_ui_Component*)psy_list_entry(p);	
		if (component->visible) {
			psy_ui_Size componentsize;
			psy_ui_Size limit;
			const psy_ui_TextMetric* c_tm;
				
			if (component->debugflag == 70) {
				self = self;
			}
			psy_ui_size_init_px(&limit,
				cp_bottomright.x - cp_topleft.x,
				cp_bottomright.y - cp_topleft.y);			
			if (!component->preventpreferredsizeatalign) {
				componentsize = psy_ui_component_preferredsize(component,
					&limit);
			} else {
				componentsize = psy_ui_component_size(component);
			}
			psy_ui_aligner_adjustminmaxsize(self, component, tm, &componentsize);
			c_tm = psy_ui_component_textmetric(self->component);
			if (component->align == psy_ui_ALIGN_CLIENT ||
				component->align == psy_ui_ALIGN_VCLIENT ||
				component->align == psy_ui_ALIGN_HCLIENT) {
				
			} else if (component->align == psy_ui_ALIGN_CENTER) {
				intptr_t centerx;
				intptr_t centery;

				centerx = (intptr_t)(floor(psy_ui_value_px(&size.width, tm)) -
					floor(psy_ui_value_px(&componentsize.width, tm))) / 2;
				centery = (intptr_t)(floor(psy_ui_value_px(&size.height, tm)) -
					floor(psy_ui_value_px(&componentsize.height, tm))) / 2;
				psy_ui_component_setposition(component,
					psy_ui_rectangle_make(
						psy_ui_point_make(
							psy_ui_value_makepx((double)centerx),
							psy_ui_value_makepx((double)centery)),
						componentsize));
			} else if (component->align == psy_ui_ALIGN_FILL) {
				psy_ui_component_setposition(component,
					psy_ui_rectangle_make(
						psy_ui_point_make(component->margin.left,
							component->margin.top),
						psy_ui_size_make(
							psy_ui_sub_values(size.width,
								psy_ui_margin_width(&component->margin, c_tm),
								c_tm),
							psy_ui_sub_values(size.height,
								psy_ui_margin_height(&component->margin, c_tm),
								c_tm))));
			} else if (component->align == psy_ui_ALIGN_TOP) {
				cp_topleft.y += floor(psy_ui_value_px(&component->margin.top, c_tm));
				cp_topleft.y += floor(psy_ui_value_px(&self->component->spacing.top, c_tm));
				psy_ui_component_setposition(component,
					psy_ui_rectangle_make(
						psy_ui_point_make(
							psy_ui_value_makepx(cp_topleft.x + floor(psy_ui_value_px(
								&component->margin.left, c_tm)) +
								floor(psy_ui_value_px(&self->component->spacing.left, c_tm))),
							psy_ui_value_makepx(cp_topleft.y)),
					psy_ui_size_make(
						psy_ui_value_makepx(cp_bottomright.x - cp_topleft.x -
							floor(psy_ui_margin_width_px(&component->margin, c_tm))),
						componentsize.height)));				
				cp_topleft.y += floor(psy_ui_value_px(&component->margin.bottom,
					c_tm));
				cp_topleft.y += floor(psy_ui_value_px(&componentsize.height, c_tm));
			} else if (component->align == psy_ui_ALIGN_BOTTOM) {				
				cp_bottomright.y -=
					floor(psy_ui_value_px(&component->margin.bottom, c_tm));
				psy_ui_component_setposition(component,
					psy_ui_rectangle_make(
					psy_ui_point_make(
						psy_ui_value_makepx(cp_topleft.x +
							floor(psy_ui_value_px(&component->margin.left, c_tm))),
						psy_ui_value_makepx(cp_bottomright.y -
							floor(psy_ui_value_px(&componentsize.height, c_tm)))),
					psy_ui_size_make(
						psy_ui_value_makepx(cp_bottomright.x - cp_topleft.x -
							floor(psy_ui_margin_width_px(&component->margin, c_tm))),
						componentsize.height)));
				cp_bottomright.y -= floor(psy_ui_value_px(&component->margin.top,
					c_tm));
				cp_bottomright.y -= floor(psy_ui_value_px(&componentsize.height,
					c_tm));
			} else if (component->align == psy_ui_ALIGN_RIGHT) {
				double requiredcomponentwidth;

				requiredcomponentwidth =
					floor(psy_ui_value_px(&componentsize.width, c_tm)) +
					floor(psy_ui_margin_width_px(&component->margin, c_tm));
				cp_bottomright.x -= floor(requiredcomponentwidth);
				psy_ui_component_setposition(component,
					psy_ui_rectangle_make(
					psy_ui_point_make(
						psy_ui_value_makepx(cp_bottomright.x +
							floor(psy_ui_value_px(&component->margin.left, c_tm))),
						psy_ui_value_makepx(cp_topleft.y +
							floor(psy_ui_value_px(&component->margin.top, c_tm)))),
					psy_ui_size_make(
						componentsize.width,
						psy_ui_value_makepx(cp_bottomright.y - cp_topleft.y -
							floor(psy_ui_margin_height_px(&component->margin,
								c_tm))))));
			} else if (component->align == psy_ui_ALIGN_LEFT) {								
				if ((self->component->alignexpandmode & psy_ui_HORIZONTALEXPAND)
						== psy_ui_HORIZONTALEXPAND) {
				} else {
					double requiredcomponentwidth;
										
					requiredcomponentwidth =
						floor(psy_ui_value_px(&componentsize.width, c_tm) +
						floor(psy_ui_margin_width_px(&component->margin, tm)));
					if (cp_topleft.x + requiredcomponentwidth >
							floor(psy_ui_value_px(&size.width, c_tm))) {						
						cp_topleft.x = 0;
						psy_ui_aligner_resizewrapline(self, wrap, cp_topleft.y,
							cpymax);
						cp_topleft.y = cpymax;
						psy_list_free(wrap);						
						wrap = 0;
					}					
					psy_list_append(&wrap, component);					
				}
				cp_topleft.x += floor(psy_ui_value_px(&component->margin.left, c_tm));				
				psy_ui_component_setposition(component,
					psy_ui_rectangle_make(
					psy_ui_point_make(
						psy_ui_value_makepx(cp_topleft.x),
						psy_ui_value_makepx(cp_topleft.y +
							floor(psy_ui_value_px(&self->component->spacing.top, c_tm)) +
							floor(psy_ui_value_px(
							&component->margin.top, c_tm)))),
					psy_ui_size_make(
						componentsize.width,
						psy_ui_value_makepx(component->justify ==
							psy_ui_JUSTIFY_EXPAND
						? cp_bottomright.y - cp_topleft.y - 
							psy_ui_margin_height_px(&self->component->spacing, c_tm) -
							floor(psy_ui_margin_height_px(&component->margin, c_tm))
						: psy_ui_value_px(&componentsize.height, c_tm)))));
				cp_topleft.x += floor(psy_ui_value_px(&component->margin.right, c_tm));
				cp_topleft.x += floor(psy_ui_value_px(&componentsize.width, c_tm));
				if (cpymax < cp_topleft.y +
						psy_ui_value_px(&componentsize.height, c_tm) +
						floor(psy_ui_margin_height_px(&component->margin, c_tm))) {
					cpymax = cp_topleft.y + floor(psy_ui_value_px(
						&componentsize.height, c_tm)) +
						floor(psy_ui_margin_height_px(&component->margin, c_tm));
				}
			} 
		}
	}
	psy_ui_align_alignclients(self, q, cp_topleft, cp_bottomright);
	psy_list_free(q);
	psy_list_free(wrap);	
}

void psy_ui_aligner_resizewrapline(psy_ui_Aligner* self, psy_List* wrap, double cpy,
	double cpymax)
{
	psy_List* w;

	for (w = wrap; w != 0; w = w->next) {
		psy_ui_Component* c;
		c = (psy_ui_Component*)w->entry;
		psy_ui_component_resize(c,
			psy_ui_size_make(
				psy_ui_component_size(c).width,
				psy_ui_value_makepx(cpymax - cpy -
					floor(psy_ui_margin_height_px(&c->margin,
						psy_ui_component_textmetric(c))))));
	}
}

void psy_ui_aligner_adjustminmaxsize(psy_ui_Aligner* self,
	psy_ui_Component* component, const psy_ui_TextMetric* tm,
	psy_ui_Size* componentsize)
{	
	if (psy_ui_component_maximumsize(component)) {	
		if (!psy_ui_value_iszero(&psy_ui_component_maximumsize(component)->width)) {
			if (psy_ui_value_comp(&psy_ui_component_maximumsize(component)->width,
					&componentsize->width, tm) < 0) {				
				componentsize->width = psy_ui_component_maximumsize(component)->width;
			}
		}	
		if (!psy_ui_value_iszero(&psy_ui_component_maximumsize(component)->height)) {
			if (psy_ui_value_comp(&psy_ui_component_maximumsize(component)->height,
					&componentsize->height, tm) < 0) {
				componentsize->height = psy_ui_component_maximumsize(component)->height;
			}
		}
	}
	if (psy_ui_component_minimumsize(component)) {
		if (!psy_ui_value_iszero(&psy_ui_component_minimumsize(component)->width)) {
			if (psy_ui_value_comp(&psy_ui_component_minimumsize(component)->width,
					&componentsize->width, tm) > 0) {
				componentsize->width = psy_ui_component_minimumsize(component)->width;
			}
		}
		if (!psy_ui_value_iszero(&psy_ui_component_minimumsize(component)->height)) {
			if (psy_ui_value_comp(&psy_ui_component_minimumsize(component)->height,
					&componentsize->height, tm) > 0) {
				componentsize->height = psy_ui_component_minimumsize(component)->height;
			}
		}
	}
}

void psy_ui_align_alignclients(psy_ui_Aligner* self, psy_List* children,
	psy_ui_RealPoint cp_topleft, psy_ui_RealPoint cp_bottomright)
{
	psy_List* p;
	uintptr_t curr;
	double height;
	double width;
	const psy_ui_TextMetric* tm;
	uintptr_t numclients;

	numclients = psy_ui_aligner_numclients(self);
	if (numclients != 0) {
		
		tm = psy_ui_component_textmetric(self->component);
		height = (double)((cp_bottomright.y - cp_topleft.y) / numclients);
		width = (double)((cp_bottomright.x - cp_topleft.x) / numclients);		
		for (curr = 0, p = children; p != NULL; p = p->next) {
			psy_ui_Component* component;			

			component = (psy_ui_Component*)p->entry;
			if (component->align == psy_ui_ALIGN_CLIENT) {
				const psy_ui_TextMetric* c_tm;

				c_tm = psy_ui_component_textmetric(self->component);
				cp_topleft.y += floor(psy_ui_value_px(&component->margin.top, c_tm));
				psy_ui_component_setposition(component,
					psy_ui_rectangle_make(
					psy_ui_point_make(
						psy_ui_value_makepx(cp_topleft.x + floor(psy_ui_value_px(
							&component->margin.left, c_tm))),
						psy_ui_value_makepx(cp_topleft.y)),
					psy_ui_size_make(
						psy_ui_value_makepx(cp_bottomright.x - cp_topleft.x -
							floor(psy_ui_margin_width_px(&component->margin, c_tm))),
						psy_ui_value_makepx((int)height))));
				cp_topleft.y += floor(psy_ui_value_px(&component->margin.bottom,
					c_tm));
				cp_topleft.y += (int)height;
				++curr;
			} else if (component->align == psy_ui_ALIGN_VCLIENT) {
				const psy_ui_TextMetric* c_tm;
				psy_ui_RealRectangle position;
				psy_ui_Size componentsize;
				psy_ui_Size limit;

				position = psy_ui_component_position(component);
				limit = psy_ui_component_size(self->component);
				c_tm = psy_ui_component_textmetric(self->component);
				componentsize = psy_ui_component_preferredsize(component,
					&limit);
				cp_topleft.y += floor(psy_ui_value_px(&component->margin.top, c_tm));
				psy_ui_component_setposition(component,
					psy_ui_rectangle_make(
					psy_ui_point_make(
						psy_ui_value_makepx(position.left + floor(psy_ui_value_px(
							&component->margin.left, c_tm))),
						psy_ui_value_makepx(cp_topleft.y)),
					psy_ui_size_make(
						psy_ui_value_makepx((int)(
							psy_ui_value_px(&componentsize.width, c_tm)) -
							floor(psy_ui_margin_width_px(&component->margin, c_tm))),
						psy_ui_value_makepx((int)height))));
				cp_topleft.y += floor(psy_ui_value_px(&component->margin.bottom,
					c_tm));
				cp_topleft.y += (int)height;
				++curr;
			} else if (component->align == psy_ui_ALIGN_HCLIENT) {
				const psy_ui_TextMetric* c_tm;
				psy_ui_RealRectangle position;
				psy_ui_Size componentsize;
				psy_ui_Size limit;

				position = psy_ui_component_position(component);
				limit = psy_ui_component_size(self->component);
				c_tm = psy_ui_component_textmetric(self->component);
				componentsize = psy_ui_component_preferredsize(component,
					&limit);
				cp_topleft.y += floor(psy_ui_value_px(&component->margin.top, c_tm));
				psy_ui_component_setposition(component,
					psy_ui_rectangle_make(
						psy_ui_point_make(
							psy_ui_value_makepx(cp_topleft.x),
							psy_ui_value_makepx(position.top + floor(psy_ui_value_px(
								&component->margin.top, c_tm)))),
						psy_ui_size_make(
							psy_ui_value_makepx((int)width),
							psy_ui_value_makepx((int)(
								psy_ui_value_px(&componentsize.height, c_tm)) -
								floor(psy_ui_margin_height_px(&component->margin, c_tm))))));
				cp_topleft.x += (int)width;
				cp_topleft.y += floor(psy_ui_value_px(&component->margin.bottom, c_tm));
				++curr;
			}
		}
	}
}

void psy_ui_aligner_adjustborder(psy_ui_Aligner* self,
	psy_ui_RealPoint* cp_topleft, psy_ui_RealPoint* cp_bottomright)
{
	const psy_ui_Border* border;

	border = psy_ui_component_border(self->component);
	if (border->left == psy_ui_BORDER_SOLID) {
		cp_topleft->x += 1;		
	}
	if (border->top == psy_ui_BORDER_SOLID) {
		cp_topleft->y += 1;		
	}
	if (border->right == psy_ui_BORDER_SOLID) {
		cp_bottomright->x -= 1;
	}
	if (border->bottom == psy_ui_BORDER_SOLID) {
		cp_bottomright->y -= 1;
	}
}

void psy_ui_aligner_adjustspacing(psy_ui_Aligner* self,
	psy_ui_RealPoint* cp_topleft, psy_ui_RealPoint* cp_bottomright)
{
	cp_topleft->x += floor(psy_ui_value_px(&self->component->spacing.left, 
		psy_ui_component_textmetric(self->component)));
}

uintptr_t psy_ui_aligner_numclients(psy_ui_Aligner* self)
{
	uintptr_t rv = 0;
	psy_List* p;
	psy_List* q;
		
	for (q = p = psy_ui_component_children(self->component, 0); p != NULL;
		p = p->next) {
		psy_ui_Component* component;

		component = (psy_ui_Component*)p->entry;
		if (component->visible) {
			if (component->align == psy_ui_ALIGN_CLIENT ||
				component->align == psy_ui_ALIGN_VCLIENT ||
				component->align == psy_ui_ALIGN_HCLIENT) {
				++rv;
			}
		}
	}
	psy_list_free(q);
	return rv;
}

void psy_ui_aligner_preferredsize(psy_ui_Aligner* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{			
	if (rv) {
		psy_ui_Size size;
		const psy_ui_TextMetric* tm;
		psy_ui_Component* client;
		psy_ui_Margin border;

		size = *rv;
		client = NULL;
		tm = psy_ui_component_textmetric(self->component);
		border = psy_ui_component_bordermargin(self->component);
		if (self->component->alignchildren &&
				!self->component->preventpreferredsize) {
			psy_List* p;
			psy_List* q;
			psy_ui_IntPoint cp;
			psy_ui_Size maxsize;
			psy_ui_IntPoint cp_topleft;
			psy_ui_IntPoint cp_bottomright;
						
			psy_ui_intpoint_init(&cp);
			psy_ui_size_init(&maxsize);			
			psy_ui_intpoint_init(&cp_topleft);
			psy_ui_intpoint_init(&cp_bottomright);			
			size.width = (!limit || (self->component->alignexpandmode &
				psy_ui_HORIZONTALEXPAND) == psy_ui_HORIZONTALEXPAND)
				? psy_ui_value_makepx(0)
				: limit->width;			
			for (p = q = psy_ui_component_children(self->component, 0);
					p != NULL; p = p->next) {
				psy_ui_Component* component;
					
				component = (psy_ui_Component*)psy_list_entry(p);				
				if (component->visible) {
					psy_ui_Size componentsize;
					psy_ui_Size limit;
					const psy_ui_TextMetric* c_tm;
					
					limit.width = psy_ui_value_makepx(psy_ui_value_px(
						&size.width, tm) - cp_topleft.x - cp_bottomright.x);
					limit.height = size.height;
					componentsize = psy_ui_component_preferredsize(component,
						&limit);					
					psy_ui_aligner_adjustminmaxsize(self, component, tm, &componentsize);
					c_tm = psy_ui_component_textmetric(component);					
					if (component->align == psy_ui_ALIGN_CLIENT) {
						if (psy_ui_value_px(&maxsize.height, tm) < cp.y +
								psy_ui_value_px(&componentsize.height, c_tm) +
							psy_ui_margin_height_px(&component->margin, c_tm))
						{
							maxsize.height = psy_ui_value_makepx(cp.y +
									psy_ui_value_px(&componentsize.height,
									c_tm) +
								psy_ui_margin_height_px(&component->margin,
									c_tm));
						}
						if (psy_ui_value_px(&maxsize.width, tm) <
								psy_ui_value_px(&componentsize.width, c_tm) +
							psy_ui_margin_width_px(&component->margin, c_tm)) {
							maxsize.width = psy_ui_value_makepx(
								psy_ui_value_px(&componentsize.width, c_tm) +
								psy_ui_margin_width_px(&component->margin,
									c_tm));
						}
					} else if (component->align == psy_ui_ALIGN_TOP ||
							component->align == psy_ui_ALIGN_BOTTOM) {
						cp.y += (intptr_t)psy_ui_value_px(&componentsize.height, c_tm) +
							(intptr_t)psy_ui_margin_height_px(&component->margin, c_tm);
						if (psy_ui_value_px(&maxsize.height, c_tm) < cp.y) {
							maxsize.height = psy_ui_value_makepx((double)cp.y);
						}
						if (psy_ui_value_px(&maxsize.width, tm) <
								psy_ui_value_px(&componentsize.width, c_tm) +
								psy_ui_margin_width_px(&component->margin,
									c_tm)) {
							maxsize.width = psy_ui_value_makepx(
								psy_ui_value_px(&componentsize.width, c_tm) +
								psy_ui_margin_width_px(&component->margin,
									c_tm));
						}
					} else if (component->align == psy_ui_ALIGN_RIGHT) {
						cp.x += (intptr_t)psy_ui_value_px(&componentsize.width, c_tm) +
							(intptr_t)psy_ui_margin_width_px(&component->margin, c_tm);
						cp_bottomright.x += (intptr_t)(psy_ui_value_px(
							&componentsize.width, c_tm) +
							psy_ui_margin_width_px(&component->margin, c_tm));
						if ((intptr_t)psy_ui_value_px(&maxsize.width, c_tm) < cp.x) {
							maxsize.width = psy_ui_value_makepx((double)cp.x);
						}
						if (psy_ui_value_px(&maxsize.height, c_tm) < cp.y +
								psy_ui_value_px(&componentsize.height, c_tm) +
								psy_ui_margin_height_px(&component->margin,
									c_tm)) {
							maxsize.height = psy_ui_value_makepx(
								cp.y + psy_ui_value_px(&componentsize.height,
									c_tm) +
								psy_ui_margin_height_px(&component->margin,
									c_tm));
						}												
					} else if (component->align == psy_ui_ALIGN_LEFT) {
						if (psy_ui_value_px(&size.width, tm) != 0) {
							intptr_t requiredcomponentwidth;

							requiredcomponentwidth = (intptr_t)psy_ui_value_px(
								&componentsize.width, c_tm) +
								(intptr_t)psy_ui_margin_width_px(&component->margin, tm);
							if (cp.x + requiredcomponentwidth >
									psy_ui_value_px(&size.width, c_tm)) {
								cp.y = (intptr_t)psy_ui_value_px(&maxsize.height, tm);
								cp.x = 0;
								cp_topleft.x = 0;
							}						
						}						
						cp.x += (intptr_t)psy_ui_value_px(&componentsize.width, c_tm) +
							(intptr_t)psy_ui_margin_width_px(&component->margin, c_tm);
						cp_topleft.x += (intptr_t)psy_ui_value_px(&componentsize.width,
								c_tm) +
							(intptr_t)psy_ui_margin_width_px(&component->margin, c_tm);
						if (psy_ui_value_px(&maxsize.width, tm) < cp.x) {
							maxsize.width = psy_ui_value_makepx((double)cp.x);
						}
						if (psy_ui_value_px(&maxsize.height, tm) < cp.y +
								psy_ui_value_px(&componentsize.height, c_tm) +
								psy_ui_margin_height_px(&component->margin,
									c_tm)) {
							maxsize.height = psy_ui_value_makepx(cp.y +
								psy_ui_value_px(&componentsize.height, c_tm) +
								psy_ui_margin_height_px(&component->margin,
									c_tm));
						}
					}				
				}
			}
			*rv = maxsize;			
			rv->width = psy_ui_value_makepx(
				floor(psy_ui_value_px(&rv->width, tm)) +
				floor(psy_ui_margin_width_px(&self->component->spacing, tm)) +
				floor(psy_ui_margin_width_px(&border, tm)));
			rv->height = psy_ui_value_makepx(psy_ui_value_px(&rv->height, tm) +
				psy_ui_margin_height_px(&self->component->spacing, tm) +
				psy_ui_margin_height_px(&border, tm));
			psy_list_free(q);				
		} else {
			*rv = size;
			rv->width = psy_ui_value_makepx(psy_ui_value_px(&rv->width, tm) +
				psy_ui_margin_width_px(&self->component->spacing, tm) +
				psy_ui_margin_height_px(&border, tm));
			rv->height = psy_ui_value_makepx(psy_ui_value_px(&rv->height, tm) +
				psy_ui_margin_width_px(&self->component->spacing, tm) +
				psy_ui_margin_height_px(&border, tm));
		}
	}	
}
