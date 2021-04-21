// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uilclaligner.h"
// ui
#include "uicomponent.h"
// std
#include <math.h>
#include <stdio.h>

// prototypes
static void psy_ui_lclaligner_align(psy_ui_LCLAligner*);
static void psy_ui_lclaligner_preferredsize(psy_ui_LCLAligner*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void psy_ui_lclaligner_adjustminmaxsize(psy_ui_LCLAligner*,
	psy_ui_Component*, const psy_ui_TextMetric*,
	psy_ui_Size*);
static uintptr_t psy_ui_lclaligner_numclients(psy_ui_LCLAligner*);
static void psy_ui_lclaligner_alignclients(psy_ui_LCLAligner*, psy_List* children,
	psy_ui_RealPoint cp_topleft, psy_ui_RealPoint cp_bottomright);
static void psy_ui_lclaligner_adjustborder(psy_ui_LCLAligner*,
	psy_ui_RealPoint* cp_topleft, psy_ui_RealPoint* cp_bottomright);
static void psy_ui_lclaligner_adjustspacing(psy_ui_LCLAligner*,
	psy_ui_RealPoint* cp_topleft, psy_ui_RealPoint* cp_bottomright);
static void psy_ui_lclaligner_resizewrapline(psy_ui_LCLAligner*, psy_List* wrap,
	double cpy, double cpymax);
static void psy_ui_lclaligner_adjustpreferredsize(psy_ui_LCLAligner* self,
	psy_ui_Size* rv);
// vtable
static psy_ui_AlignerVTable lclaligner_vtable;
static bool lclaligner_vtable_initialized = FALSE;

static void lclaligner_vtable_init(psy_ui_LCLAligner* self)
{
	if (!lclaligner_vtable_initialized) {
		lclaligner_vtable = *(self->aligner.vtable);		
		lclaligner_vtable.align =
			(psy_ui_fp_aligner_align)
			psy_ui_lclaligner_align;
		lclaligner_vtable.preferredsize =
			(psy_ui_fp_aligner_preferredsize)
			psy_ui_lclaligner_preferredsize;
		lclaligner_vtable_initialized = TRUE;
	}
	self->aligner.vtable = &lclaligner_vtable;
}
// implementation
void psy_ui_lclaligner_init(psy_ui_LCLAligner* self, psy_ui_Component* component)
{
	psy_ui_aligner_init(&self->aligner);
	lclaligner_vtable_init(self);
	self->component = component;
}

void psy_ui_lclaligner_align(psy_ui_LCLAligner* self)
{	
	psy_ui_Size size;
	const psy_ui_TextMetric* tm;
	psy_ui_RealPoint cp_topleft;
	psy_ui_RealPoint cp_bottomright;
	double cpymax = 0;
	psy_List* p;
	psy_List* q;
	psy_List* wrap = 0;	
	
	size = psy_ui_component_offsetsize(self->component);
	tm = psy_ui_component_textmetric(self->component);
	psy_ui_realpoint_init(&cp_topleft);
	psy_ui_realpoint_init_all(&cp_bottomright,
		psy_ui_value_px(&size.width, tm),
		psy_ui_value_px(&size.height, tm));
	psy_ui_lclaligner_adjustborder(self, &cp_topleft, &cp_bottomright);
	psy_ui_lclaligner_adjustspacing(self, &cp_topleft, &cp_bottomright);
	for (p = q = psy_ui_component_children(self->component, 0); p != NULL;
			psy_list_next(&p)) {
		psy_ui_Component* component;
			
		component = (psy_ui_Component*)psy_list_entry(p);
		if (component->visible) {
			psy_ui_Size componentsize;
			psy_ui_Size limit;
			psy_ui_Margin c_margin;			
			const psy_ui_TextMetric* c_tm;
				
			psy_ui_size_init_px(&limit,
				cp_bottomright.x - cp_topleft.x,
				cp_bottomright.y - cp_topleft.y);			
			if (!component->preventpreferredsizeatalign) {
				componentsize = psy_ui_component_preferredsize(component,
					&limit);
			} else {
				componentsize = psy_ui_component_offsetsize(component);
			}			
			psy_ui_lclaligner_adjustminmaxsize(self, component, tm, &componentsize);
			c_tm = psy_ui_component_textmetric(component);
			c_margin = psy_ui_component_margin(component);			
			if (component->align == psy_ui_ALIGN_CLIENT ||
				component->align == psy_ui_ALIGN_VCLIENT ||
				component->align == psy_ui_ALIGN_HCLIENT) {
				
			} else if (component->align == psy_ui_ALIGN_FIXED_RESIZE) {
				psy_ui_RealRectangle position;						

				position = psy_ui_component_position(component);
				psy_ui_component_setposition(component,
					psy_ui_rectangle_make(
						psy_ui_point_make(
							psy_ui_value_make_px(position.left),
							psy_ui_value_make_px(position.top)),
						componentsize));	
			} else if (component->align == psy_ui_ALIGN_CENTER) {
				intptr_t centerx;
				intptr_t centery;

				centerx = (intptr_t)(psy_ui_value_px(&size.width, tm) -
					psy_ui_value_px(&componentsize.width, tm)) / 2;
				centery = (intptr_t)(psy_ui_value_px(&size.height, tm) -
					psy_ui_value_px(&componentsize.height, tm)) / 2;
				psy_ui_component_setposition(component,
					psy_ui_rectangle_make(
						psy_ui_point_make(
							psy_ui_value_make_px((double)centerx),
							psy_ui_value_make_px((double)centery)),
						componentsize));				
			} else if (component->align == psy_ui_ALIGN_TOP) {
				cp_topleft.y += psy_ui_value_px(&c_margin.top, c_tm);
				psy_ui_component_setposition(component,
					psy_ui_rectangle_make(
						psy_ui_point_make(
							psy_ui_value_make_px(cp_topleft.x + psy_ui_value_px(
								&c_margin.left, c_tm)),
							psy_ui_value_make_px(cp_topleft.y)),
					psy_ui_size_make(
						psy_ui_value_make_px(cp_bottomright.x - cp_topleft.x -
							psy_ui_margin_width_px(&c_margin, c_tm)),
						componentsize.height)));
				cp_topleft.y += psy_ui_value_px(&c_margin.bottom, c_tm);
				cp_topleft.y += psy_ui_value_px(&componentsize.height, c_tm);
			} else if (component->align == psy_ui_ALIGN_BOTTOM) {				
				cp_bottomright.y -= psy_ui_value_px(&c_margin.bottom, c_tm);
				psy_ui_component_setposition(component,
					psy_ui_rectangle_make(
					psy_ui_point_make(
						psy_ui_value_make_px(cp_topleft.x +
							psy_ui_value_px(&c_margin.left, c_tm)),
						psy_ui_value_make_px(cp_bottomright.y -
							psy_ui_value_px(&componentsize.height, c_tm))),
					psy_ui_size_make(
						psy_ui_value_make_px(cp_bottomright.x - cp_topleft.x -
							psy_ui_margin_width_px(&c_margin, c_tm)),
						componentsize.height)));
				cp_bottomright.y -= psy_ui_value_px(&c_margin.top, c_tm);
				cp_bottomright.y -= psy_ui_value_px(&componentsize.height, c_tm);
			} else if (component->align == psy_ui_ALIGN_RIGHT) {
				double requiredcomponentwidth;

				requiredcomponentwidth =
					psy_ui_value_px(&componentsize.width, c_tm) +
					psy_ui_margin_width_px(&c_margin, c_tm);
				cp_bottomright.x -= requiredcomponentwidth;
				psy_ui_component_setposition(component,
					psy_ui_rectangle_make(
					psy_ui_point_make(
						psy_ui_value_make_px(cp_bottomright.x +
							psy_ui_value_px(&c_margin.left, c_tm)),
						psy_ui_value_make_px(cp_topleft.y +
							psy_ui_value_px(&c_margin.top, c_tm))),
					psy_ui_size_make(
						componentsize.width,
						psy_ui_value_make_px(cp_bottomright.y - cp_topleft.y -
							psy_ui_margin_height_px(&c_margin, c_tm)))));
			} else if (component->align == psy_ui_ALIGN_LEFT) {								
				if ((self->component->alignexpandmode & psy_ui_HORIZONTALEXPAND)
						== psy_ui_HORIZONTALEXPAND) {
				} else {
					double requiredcomponentwidth;
										
					requiredcomponentwidth =
						psy_ui_value_px(&componentsize.width, c_tm) +
						psy_ui_margin_width_px(&c_margin, tm);
					if (cp_topleft.x + requiredcomponentwidth >
							psy_ui_value_px(&size.width, c_tm)) {
						cp_topleft.x = 0;
						psy_ui_lclaligner_resizewrapline(self, wrap, cp_topleft.y,
							cpymax);
						cp_topleft.y = cpymax;
						psy_list_free(wrap);						
						wrap = 0;
					}					
					psy_list_append(&wrap, component);					
				}
				cp_topleft.x += psy_ui_value_px(&c_margin.left, c_tm);
				psy_ui_component_setposition(component,
					psy_ui_rectangle_make(
					psy_ui_point_make(
						psy_ui_value_make_px(cp_topleft.x),
						psy_ui_value_make_px(cp_topleft.y +	
							psy_ui_value_px(&c_margin.top, c_tm))),
					psy_ui_size_make(
						componentsize.width,
						psy_ui_value_make_px(
							cp_bottomright.y - cp_topleft.y - 		
							psy_ui_margin_height_px(&c_margin, c_tm)))));
				cp_topleft.x += psy_ui_value_px(&c_margin.right, c_tm);
				cp_topleft.x += psy_ui_value_px(&componentsize.width, c_tm);
				if (cpymax < cp_topleft.y +
						psy_ui_value_px(&componentsize.height, c_tm) +
						psy_ui_margin_height_px(&c_margin, c_tm)) {
					cpymax = cp_topleft.y + psy_ui_value_px(
						&componentsize.height, c_tm) +
						psy_ui_margin_height_px(&c_margin, c_tm);
				}
			} 
		}
	}
	psy_ui_lclaligner_alignclients(self, q, cp_topleft, cp_bottomright);
	psy_list_free(q);
	psy_list_free(wrap);	
}

void psy_ui_lclaligner_resizewrapline(psy_ui_LCLAligner* self, psy_List* wrap, double cpy,
	double cpymax)
{
	psy_List* w;

	for (w = wrap; w != 0; w = w->next) {
		psy_ui_Component* c;
		c = (psy_ui_Component*)w->entry;
		psy_ui_Margin c_margin;

		c_margin = psy_ui_component_margin(c);
		psy_ui_component_resize(c,
			psy_ui_size_make(
				psy_ui_component_offsetsize(c).width,
				psy_ui_value_make_px(cpymax - cpy -
					psy_ui_margin_height_px(&c_margin,
						psy_ui_component_textmetric(c)))));
	}
}

void psy_ui_lclaligner_adjustminmaxsize(psy_ui_LCLAligner* self,
	psy_ui_Component* component, const psy_ui_TextMetric* tm,
	psy_ui_Size* componentsize)
{	
	psy_ui_Size minsize;
	psy_ui_Size maxsize;

	minsize = psy_ui_component_minimumsize(component);
	maxsize = psy_ui_component_maximumsize(component);
	if (!psy_ui_size_iszero(&maxsize)) {
		if (!psy_ui_value_iszero(&maxsize.width)) {
			if (psy_ui_value_comp(&maxsize.width,
					&componentsize->width, tm) < 0) {				
				componentsize->width = maxsize.width;
			}
		}	
		if (!psy_ui_value_iszero(&maxsize.height)) {
			if (psy_ui_value_comp(&maxsize.height,
					&componentsize->height, tm) < 0) {
				componentsize->height = maxsize.height;
			}
		}
	}
	if (!psy_ui_size_iszero(&minsize)) {
		if (!psy_ui_value_iszero(&minsize.width)) {
			if (psy_ui_value_comp(&minsize.width,
					&componentsize->width, tm) > 0) {
				componentsize->width = minsize.width;
			}
		}
		if (!psy_ui_value_iszero(&minsize.height)) {
			if (psy_ui_value_comp(&minsize.height,
					&componentsize->height, tm) > 0) {
				componentsize->height = minsize.height;
			}
		}
	}
}

void psy_ui_lclaligner_alignclients(psy_ui_LCLAligner* self, psy_List* children,
	psy_ui_RealPoint cp_topleft, psy_ui_RealPoint cp_bottomright)
{
	psy_List* p;
	uintptr_t curr;
	double height;
	double width;
	const psy_ui_TextMetric* tm;
	uintptr_t numclients;

	numclients = psy_ui_lclaligner_numclients(self);
	if (numclients != 0) {
		
		tm = psy_ui_component_textmetric(self->component);
		height = (double)((cp_bottomright.y - cp_topleft.y) / numclients);
		width = (double)((cp_bottomright.x - cp_topleft.x) / numclients);		
		for (curr = 0, p = children; p != NULL; p = p->next) {
			psy_ui_Component* component;
			psy_ui_Margin c_margin;			

			component = (psy_ui_Component*)p->entry;
			if (component->visible) {
				c_margin = psy_ui_component_margin(component);				
				if (component->align == psy_ui_ALIGN_CLIENT) {
					const psy_ui_TextMetric* c_tm;

					c_tm = psy_ui_component_textmetric(self->component);
					cp_topleft.y += psy_ui_value_px(&c_margin.top, c_tm);
					psy_ui_component_setposition(component,
						psy_ui_rectangle_make(
							psy_ui_point_make(
								psy_ui_value_make_px(cp_topleft.x + psy_ui_value_px(
									&c_margin.left, c_tm)),
								psy_ui_value_make_px(cp_topleft.y)),
							psy_ui_size_make(
								psy_ui_value_make_px(cp_bottomright.x - cp_topleft.x -
									psy_ui_margin_width_px(&c_margin, c_tm)),
								psy_ui_value_make_px((int)height))));
					cp_topleft.y += psy_ui_value_px(&c_margin.bottom, c_tm);
					cp_topleft.y += (int)height;
					++curr;
				} else if (component->align == psy_ui_ALIGN_VCLIENT) {
					const psy_ui_TextMetric* c_tm;
					psy_ui_RealRectangle position;
					psy_ui_Size componentsize;
					psy_ui_Size limit;

					position = psy_ui_component_position(component);
					limit = psy_ui_component_offsetsize(self->component);
					c_tm = psy_ui_component_textmetric(self->component);
					componentsize = psy_ui_component_preferredsize(component,
						&limit);
					cp_topleft.y += psy_ui_value_px(&c_margin.top, c_tm);
					psy_ui_component_setposition(component,
						psy_ui_rectangle_make(
							psy_ui_point_make(
								psy_ui_value_make_px(position.left + psy_ui_value_px(
									&c_margin.left, c_tm)),
								psy_ui_value_make_px(cp_topleft.y)),
							psy_ui_size_make(
								psy_ui_value_make_px((int)(
									psy_ui_value_px(&componentsize.width, c_tm)) -
									psy_ui_margin_width_px(&c_margin, c_tm)),
								psy_ui_value_make_px((int)height))));
					cp_topleft.y += psy_ui_value_px(&c_margin.bottom, c_tm);
					cp_topleft.y += (int)height;
					++curr;
				} else if (component->align == psy_ui_ALIGN_HCLIENT) {
					const psy_ui_TextMetric* c_tm;
					psy_ui_RealRectangle position;
					psy_ui_Size componentsize;
					psy_ui_Size limit;

					position = psy_ui_component_position(component);
					limit = psy_ui_component_offsetsize(self->component);
					c_tm = psy_ui_component_textmetric(self->component);
					componentsize = psy_ui_component_preferredsize(component,
						&limit);
					cp_topleft.y += psy_ui_value_px(&c_margin.top, c_tm);
					psy_ui_component_setposition(component,
						psy_ui_rectangle_make(
							psy_ui_point_make(
								psy_ui_value_make_px(cp_topleft.x),
								psy_ui_value_make_px(position.top + psy_ui_value_px(
									&c_margin.top, c_tm))),
							psy_ui_size_make(
								psy_ui_value_make_px((int)width),
								psy_ui_value_make_px((int)(
									psy_ui_value_px(&componentsize.height, c_tm)) -
									psy_ui_margin_height_px(&c_margin, c_tm)))));
					cp_topleft.x += (int)width;
					cp_topleft.y += psy_ui_value_px(&c_margin.bottom, c_tm);
					++curr;
				}
			}
		}
	}
}

void psy_ui_lclaligner_adjustborder(psy_ui_LCLAligner* self,
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

void psy_ui_lclaligner_adjustspacing(psy_ui_LCLAligner* self,
	psy_ui_RealPoint* cp_topleft, psy_ui_RealPoint* cp_bottomright)
{
	psy_ui_Margin spacing;

	spacing = psy_ui_component_spacing(self->component);
	cp_topleft->x += psy_ui_value_px(&spacing.left, 
		psy_ui_component_textmetric(self->component));
	cp_topleft->y += psy_ui_value_px(&spacing.top,
		psy_ui_component_textmetric(self->component));
	cp_bottomright->x -= psy_ui_value_px(&spacing.right,
		psy_ui_component_textmetric(self->component));
	cp_bottomright->x = psy_max(0.0, cp_bottomright->x);
	cp_bottomright->y -= psy_ui_value_px(&spacing.bottom,
		psy_ui_component_textmetric(self->component));
	cp_bottomright->y = psy_max(0.0, cp_bottomright->y);
}

uintptr_t psy_ui_lclaligner_numclients(psy_ui_LCLAligner* self)
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

void psy_ui_lclaligner_preferredsize(psy_ui_LCLAligner* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{			
	if (rv) {
		psy_ui_Size size;
		const psy_ui_TextMetric* tm;
		psy_ui_Component* client;
		psy_ui_Margin margin;		
				
		size = *rv;
		client = NULL;
		tm = psy_ui_component_textmetric(self->component);
		margin = psy_ui_component_margin(self->component);
		if (self->component->containeralign != psy_ui_CONTAINER_ALIGN_NONE &&
				!self->component->preventpreferredsize) {
			psy_List* p;
			psy_List* q;
			psy_ui_RealPoint cp;
			psy_ui_Size maxsize;
			psy_ui_RealPoint cp_topleft;
			psy_ui_RealPoint cp_bottomright;
						
			psy_ui_realpoint_init(&cp);
			psy_ui_size_init(&maxsize);			
			psy_ui_realpoint_init(&cp_topleft);
			psy_ui_realpoint_init(&cp_bottomright);			
			size.width = (!limit || (self->component->alignexpandmode &
				psy_ui_HORIZONTALEXPAND) == psy_ui_HORIZONTALEXPAND)
				? psy_ui_value_make_px(0)
				: limit->width;			
			for (p = q = psy_ui_component_children(self->component, 0);
					p != NULL; p = p->next) {
				psy_ui_Component* component;
					
				component = (psy_ui_Component*)psy_list_entry(p);				
				if (component->visible) {
					psy_ui_Size componentsize;
					psy_ui_Size limit;
					const psy_ui_TextMetric* c_tm;
					psy_ui_Margin c_margin;
					psy_ui_Margin c_spacing;

					c_margin = psy_ui_component_margin(component);
					c_spacing = psy_ui_component_spacing(component);
					limit.width = psy_ui_value_make_px(psy_ui_value_px(
						&size.width, tm) - cp_topleft.x - cp_bottomright.x);
					limit.height = size.height;
					componentsize = psy_ui_component_preferredsize(component,
						&limit);					
					psy_ui_lclaligner_adjustminmaxsize(self, component, tm, &componentsize);
					c_tm = psy_ui_component_textmetric(component);		
					if (component->align == psy_ui_ALIGN_FIXED_RESIZE) {
						psy_ui_RealRectangle position;

						position = psy_ui_component_position(component);
						if (psy_ui_value_px(&maxsize.height, tm) <
							position.top +
							psy_ui_value_px(&componentsize.height, c_tm) +
							psy_ui_margin_height_px(&c_margin, c_tm))
						{
							maxsize.height = psy_ui_value_make_px(position.top +
								psy_ui_value_px(&componentsize.height,
									c_tm) +
								psy_ui_margin_height_px(&c_margin,
									c_tm));
						}
						if (psy_ui_value_px(&maxsize.width, tm) <
								position.left +
								psy_ui_value_px(&componentsize.width, c_tm) +
								psy_ui_margin_width_px(&c_margin, c_tm)) {
							maxsize.width = psy_ui_value_make_px(
								position.left +
								psy_ui_value_px(&componentsize.width, c_tm) +
								psy_ui_margin_width_px(&c_margin, c_tm));
						}
					} else if (component->align == psy_ui_ALIGN_CLIENT ||
							component->align == psy_ui_ALIGN_CENTER) {
						if (psy_ui_value_px(&maxsize.height, tm) < cp.y +
								psy_ui_value_px(&componentsize.height, c_tm) +
							psy_ui_margin_height_px(&c_margin, c_tm))
						{
							maxsize.height = psy_ui_value_make_px(cp.y +
									psy_ui_value_px(&componentsize.height,
									c_tm) +
								psy_ui_margin_height_px(&c_margin,
									c_tm));
						}
						if (psy_ui_value_px(&maxsize.width, tm) <
								psy_ui_value_px(&componentsize.width, c_tm) +
							psy_ui_margin_width_px(&c_margin, c_tm)) {
							maxsize.width = psy_ui_value_make_px(
								psy_ui_value_px(&componentsize.width, c_tm) +
								psy_ui_margin_width_px(&c_margin, c_tm));
						}
					} else if (component->align == psy_ui_ALIGN_TOP ||
							component->align == psy_ui_ALIGN_BOTTOM) {
						cp.y += psy_ui_value_px(&componentsize.height, c_tm) +
							psy_ui_margin_height_px(&c_margin, c_tm);
						if (psy_ui_value_px(&maxsize.height, c_tm) < cp.y) {
							maxsize.height = psy_ui_value_make_px(cp.y);
						}
						if (psy_ui_value_px(&maxsize.width, tm) <
								psy_ui_value_px(&componentsize.width, c_tm) +
								psy_ui_margin_width_px(&c_margin,
									c_tm)) {
							maxsize.width = psy_ui_value_make_px(
								psy_ui_value_px(&componentsize.width, c_tm) +
								psy_ui_margin_width_px(&c_margin, c_tm));
						}
					} else if (component->align == psy_ui_ALIGN_RIGHT) {
						cp.x += psy_ui_value_px(&componentsize.width, c_tm) +
							psy_ui_margin_width_px(&c_margin, c_tm);
						cp_bottomright.x += (intptr_t)(psy_ui_value_px(
							&componentsize.width, c_tm) +
							psy_ui_margin_width_px(&c_margin, c_tm));
						if (psy_ui_value_px(&maxsize.width, c_tm) < cp.x) {
							maxsize.width = psy_ui_value_make_px(cp.x);
						}
						if (psy_ui_value_px(&maxsize.height, c_tm) < cp.y +
								psy_ui_value_px(&componentsize.height, c_tm) +
								psy_ui_margin_height_px(&c_margin, c_tm)) {
							maxsize.height = psy_ui_value_make_px(
								cp.y + psy_ui_value_px(&componentsize.height,
									c_tm) +
								psy_ui_margin_height_px(&c_margin, c_tm));
						}												
					} else if (component->align == psy_ui_ALIGN_LEFT) {
						if (psy_ui_value_px(&size.width, tm) != 0) {
							intptr_t requiredcomponentwidth;

							requiredcomponentwidth = (intptr_t)psy_ui_value_px(
								&componentsize.width, c_tm) +
								(intptr_t)psy_ui_margin_width_px(&c_margin, tm);
							if (cp.x + requiredcomponentwidth >
									psy_ui_value_px(&size.width, c_tm)) {
								cp.y = (intptr_t)psy_ui_value_px(&maxsize.height, tm);
								cp.x = 0;
								cp_topleft.x = 0;
							}						
						}						
						cp.x += psy_ui_value_px(&componentsize.width, c_tm) +
							psy_ui_margin_width_px(&c_margin, c_tm);
						cp_topleft.x += (intptr_t)psy_ui_value_px(&componentsize.width,
							c_tm) +
							psy_ui_margin_width_px(&c_margin, c_tm);
						if (psy_ui_value_px(&maxsize.width, tm) < cp.x) {
							maxsize.width = psy_ui_value_make_px((double)cp.x);
						}
						if (psy_ui_value_px(&maxsize.height, tm) < cp.y +
								psy_ui_value_px(&componentsize.height, c_tm) +
								psy_ui_margin_height_px(&c_margin, c_tm)) {
							maxsize.height = psy_ui_value_make_px(cp.y +
								psy_ui_value_px(&componentsize.height, c_tm) +
								psy_ui_margin_height_px(&c_margin, c_tm));
						}
					}				
				}
			}
			psy_list_free(q);
			*rv = maxsize;			
		} else {
			*rv = size;			
		}
		psy_ui_lclaligner_adjustpreferredsize(self, rv);
	}	
}

void psy_ui_lclaligner_adjustpreferredsize(psy_ui_LCLAligner* self,
	psy_ui_Size* rv)
{
	const psy_ui_TextMetric* tm;
	psy_ui_RealMargin border;
	psy_ui_RealMargin spacing;

	tm = psy_ui_component_textmetric(self->component);
	spacing = psy_ui_component_spacing_px(self->component);	
	border = psy_ui_component_bordermargin_px(self->component);
	psy_ui_size_setpx(rv,
		psy_ui_value_px(&rv->width, tm) +
			psy_ui_realmargin_width(&spacing) +
			psy_ui_realmargin_width(&border),
		psy_ui_value_px(&rv->height, tm) +
			psy_ui_realmargin_height(&spacing) +
			psy_ui_realmargin_height(&border));	
}
