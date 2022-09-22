/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uilclaligner.h"
/* local */
#include "uicomponent.h"
/* std */
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
/* platform */
#include "../../detail/trace.h"

/* prototypes */
static void psy_ui_lclaligner_align(psy_ui_LCLAligner*, psy_ui_Component* group);
static void psy_ui_lclaligner_preferred_size(psy_ui_LCLAligner*, psy_ui_Component* group,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_lclaligner_align_clients(psy_ui_LCLAligner*, psy_ui_Component* group,
	uintptr_t numclients, psy_List* children,
	psy_ui_RealPoint cp_topleft, psy_ui_RealPoint cp_bottomright);
static void psy_ui_lclaligner_resize_wrap_line(psy_ui_LCLAligner*, psy_List* wrap,
	double cpy, double cpymax);

/* vtable */
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
			psy_ui_lclaligner_preferred_size;
		lclaligner_vtable_initialized = TRUE;
	}
	self->aligner.vtable = &lclaligner_vtable;
}

/* implementation */
void psy_ui_lclaligner_init(psy_ui_LCLAligner* self)
{
	psy_ui_aligner_init(&self->aligner);
	lclaligner_vtable_init(self);	
}

psy_ui_LCLAligner* psy_ui_lclaligner_alloc(void)
{
	return (psy_ui_LCLAligner*)malloc(sizeof(psy_ui_LCLAligner));
}

psy_ui_LCLAligner* psy_ui_lclaligner_allocinit(void)
{
	psy_ui_LCLAligner* rv;

	rv = psy_ui_lclaligner_alloc();
	if (rv) {
		psy_ui_lclaligner_init(rv);
	}
	return rv;
}

void psy_ui_lclaligner_align(psy_ui_LCLAligner* self, psy_ui_Component* group)
{	
	psy_ui_Size size;
	psy_ui_Size parentsize;
	const psy_ui_TextMetric* tm;
	psy_ui_RealPoint cp_topleft;
	psy_ui_RealPoint cp_topleft_wrapstart;
	psy_ui_RealPoint cp_bottomright;
	uintptr_t numclients;
	double cpymax = 0;
	psy_List* p;
	psy_List* q;
	psy_List* wrap = 0;	
	
	assert(group);

	size = psy_ui_component_scroll_size(group);
	if (psy_ui_component_parent(group)) {
		parentsize = psy_ui_component_scroll_size(
			psy_ui_component_parent(group));
	} else {
		parentsize = size;
	}
	tm = psy_ui_component_textmetric(group);	
	psy_ui_realpoint_init(&cp_topleft);
	psy_ui_realpoint_init_all(&cp_bottomright,
		psy_ui_value_px(&size.width, tm, &parentsize),
		psy_ui_value_px(&size.height, tm, &parentsize));
	psy_ui_aligner_adjust_border(group, &cp_topleft,
		&cp_bottomright);
	psy_ui_aligner_adjust_spacing(group, &cp_topleft,
		&cp_bottomright);
	cp_topleft_wrapstart = cp_topleft;
	numclients = 0;
	for (p = q = psy_ui_component_children(group, 0); p != NULL;
			psy_list_next(&p)) {
		psy_ui_Component* curr;
			
		curr = (psy_ui_Component*)psy_list_entry(p);
		if (psy_ui_component_visible(curr)) {
			psy_ui_Size componentsize;
			psy_ui_Size limit;
			psy_ui_Margin c_margin;			
			const psy_ui_TextMetric* c_tm;
				
			psy_ui_size_init_px(&limit,
				cp_bottomright.x - cp_topleft.x,
				cp_bottomright.y - cp_topleft.y);
			psy_ui_aligner_adjust_margin_size(curr, &size, &limit);
			componentsize = psy_ui_component_preferred_size(curr, &limit);
			psy_ui_aligner_adjust_minmax_size(curr, tm, &componentsize,
				&size);
			c_tm = psy_ui_component_textmetric(curr);			
			c_margin = psy_ui_component_margin(curr);
			if (curr->align == psy_ui_ALIGN_CLIENT ||
				curr->align == psy_ui_ALIGN_VCLIENT ||
				curr->align == psy_ui_ALIGN_HCLIENT) {
					++numclients;
			} else if (curr->align == psy_ui_ALIGN_FIXED) {
				psy_ui_RealRectangle position;						

				position = psy_ui_component_position(curr);
				psy_ui_component_setposition(curr,
					psy_ui_rectangle_make(
						psy_ui_point_make_px(position.left, position.top),
						componentsize));	
			} else if (curr->align == psy_ui_ALIGN_CENTER) {
				psy_ui_RealPoint center;				
								
				center.x = cp_topleft.x + floor((cp_bottomright.x - cp_topleft.x -
						psy_ui_margin_width_px(&c_margin, c_tm, &size) -
						psy_ui_value_px(&componentsize.width, tm, &size)) / 2.0) +
						psy_ui_value_px(&c_margin.left, c_tm, &size);
				center.y = cp_topleft.y + floor((cp_bottomright.y - cp_topleft.y -
					psy_ui_margin_height_px(&c_margin, c_tm, &size) -
					psy_ui_value_px(&componentsize.height, tm, &size)) / 2.0) +
					psy_ui_value_px(&c_margin.top, c_tm, &size);
				psy_ui_component_setposition(curr,
					psy_ui_rectangle_make(
						psy_ui_point_make_px(center.x, center.y),
						componentsize));
			} else if (curr->align == psy_ui_ALIGN_TOP) {
				cp_topleft.y += psy_ui_value_px(&c_margin.top, c_tm, &size);
				psy_ui_component_setposition(curr,
					psy_ui_rectangle_make(
						psy_ui_point_make(
							psy_ui_value_make_px(cp_topleft.x + psy_ui_value_px(
								&c_margin.left, c_tm, &size)),
							psy_ui_value_make_px(cp_topleft.y)),
					psy_ui_size_make(
						psy_ui_value_make_px(cp_bottomright.x - cp_topleft.x -
							psy_ui_margin_width_px(&c_margin, c_tm, &size)),
						componentsize.height)));
				cp_topleft.y += psy_ui_value_px(&c_margin.bottom, c_tm, &size);
				cp_topleft.y += psy_ui_value_px(&componentsize.height, c_tm, &size);
			} else if (curr->align == psy_ui_ALIGN_BOTTOM) {
				double compheight;

				cp_bottomright.y -= psy_ui_value_px(&c_margin.bottom, c_tm, &size);				
				compheight = psy_ui_value_px(&componentsize.height, c_tm, &size);
				psy_ui_component_setposition(curr,
					psy_ui_rectangle_make(
					psy_ui_point_make_px(
						cp_topleft.x + psy_ui_value_px(&c_margin.left, c_tm, &size),
						cp_bottomright.y - compheight),
					psy_ui_size_make_px(
						cp_bottomright.x - cp_topleft.x -
							psy_ui_margin_width_px(&c_margin, c_tm, &size),
						compheight)));
				cp_bottomright.y -= psy_ui_value_px(&c_margin.top, c_tm, &size);
				cp_bottomright.y -= compheight;
			} else if (curr->align == psy_ui_ALIGN_RIGHT) {
				double requiredcomponentwidth;

				requiredcomponentwidth =
					psy_ui_value_px(&componentsize.width, c_tm, &size) +
					psy_ui_margin_width_px(&c_margin, c_tm, &size);
				cp_bottomright.x -= requiredcomponentwidth;
				psy_ui_component_setposition(curr,
					psy_ui_rectangle_make(
					psy_ui_point_make_px(
							cp_bottomright.x + psy_ui_value_px(&c_margin.left, c_tm, &size),
							cp_topleft.y + psy_ui_value_px(&c_margin.top, c_tm, &size)),
					psy_ui_size_make(
						componentsize.width,
						psy_ui_value_make_px(cp_bottomright.y - cp_topleft.y -
							psy_ui_margin_height_px(&c_margin, c_tm, &size)))));
			} else if (curr->align == psy_ui_ALIGN_LEFT) {				
				if ((self->aligner.alignexpandmode & psy_ui_HEXPAND)
						== psy_ui_HEXPAND) {
				} else {
					double requiredcomponentwidth;
										
					requiredcomponentwidth =
						psy_ui_value_px(&componentsize.width, c_tm, &size) +
						psy_ui_margin_width_px(&c_margin, tm, &size);
					if (cp_topleft.x + requiredcomponentwidth >
							psy_ui_value_px(&size.width, c_tm, &size)) {						
						cp_topleft.x = cp_topleft_wrapstart.x;
						psy_ui_lclaligner_resize_wrap_line(self, wrap, cp_topleft.y,
							cpymax);
						cp_topleft.y = cpymax;
						psy_list_free(wrap);						
						wrap = 0;
						psy_ui_size_init_px(&limit,
							cp_bottomright.x - cp_topleft.x,
							cp_bottomright.y - cp_topleft.y);
						componentsize = psy_ui_component_preferred_size(curr,
							&limit);
					}		
					psy_list_append(&wrap, curr);
				}
				cp_topleft.x += psy_ui_value_px(&c_margin.left, c_tm,
					&size);				
				psy_ui_component_setposition(curr,
					psy_ui_rectangle_make(
						psy_ui_point_make_px(
							cp_topleft.x,
							cp_topleft.y +	psy_ui_value_px(&c_margin.top,
							c_tm, &size)),
						psy_ui_size_make(
							componentsize.width,
							psy_ui_value_make_px(
								cp_bottomright.y - cp_topleft.y -
								psy_ui_margin_height_px(&c_margin,
								c_tm, &size)))));
				cp_topleft.x += psy_ui_value_px(&c_margin.right, c_tm, &size);
				cp_topleft.x += psy_ui_value_px(&componentsize.width, c_tm, &size);
				if (cpymax < cp_topleft.y +
						psy_ui_value_px(&componentsize.height, c_tm, &size) +
						psy_ui_margin_height_px(&c_margin, c_tm, &size)) {
					cpymax = cp_topleft.y + psy_ui_value_px(
						&componentsize.height, c_tm, &size) +
						psy_ui_margin_height_px(&c_margin, c_tm, &size);
				}
			} 
		}
	}
	psy_ui_lclaligner_align_clients(self, group, numclients, q, cp_topleft, cp_bottomright);
	psy_list_free(q);
	psy_list_free(wrap);	
}

void psy_ui_lclaligner_resize_wrap_line(psy_ui_LCLAligner* self, psy_List* wrap, double cpy,
	double cpymax)
{
	psy_List* w;

	for (w = wrap; w != 0; w = w->next) {
		psy_ui_Component* c;
		psy_ui_Margin c_margin;

		c = (psy_ui_Component*)w->entry;
		c_margin = psy_ui_component_margin(c);
		psy_ui_component_resize(c,
			psy_ui_size_make(
				psy_ui_component_scroll_size(c).width,
				psy_ui_value_make_px(cpymax - cpy -
					psy_ui_margin_height_px(&c_margin,
						psy_ui_component_textmetric(c), NULL))));
	}
}

void psy_ui_lclaligner_align_clients(psy_ui_LCLAligner* self, psy_ui_Component* group,
	uintptr_t numclients, psy_List* children, psy_ui_RealPoint cp_topleft,
	psy_ui_RealPoint cp_bottomright)
{
	psy_List* p;	
	double height;
	double width;
	const psy_ui_TextMetric* tm;	

	if (numclients == 0) {
		return;
	}	
	tm = psy_ui_component_textmetric(group);
	height = (double)((cp_bottomright.y - cp_topleft.y) / numclients);
	width = (double)((cp_bottomright.x - cp_topleft.x) / numclients);		
	for (p = children; p != NULL; p = p->next) {
		psy_ui_Component* curr;
		psy_ui_Margin c_margin;			

		curr = (psy_ui_Component*)p->entry;			
		if (psy_ui_component_visible(curr)) {
			c_margin = psy_ui_component_margin(curr);
			if (curr->align == psy_ui_ALIGN_CLIENT) {
				const psy_ui_TextMetric* c_tm;
				psy_ui_Size parentsize;
					
				parentsize = psy_ui_size_make_px(
					cp_bottomright.x - cp_topleft.x,
					cp_bottomright.y - cp_topleft.y);
				c_tm = tm;
				cp_topleft.y += psy_ui_value_px(&c_margin.top, c_tm, &parentsize);
				psy_ui_component_setposition(curr,
					psy_ui_rectangle_make(
						psy_ui_point_make_px(
							cp_topleft.x + psy_ui_value_px(&c_margin.left, c_tm, &parentsize),
							cp_topleft.y + psy_ui_value_px(&c_margin.top, c_tm, &parentsize)),
						psy_ui_size_make_px(
							cp_bottomright.x - cp_topleft.x - psy_ui_margin_width_px(&c_margin, c_tm, &parentsize),
							(int)height - psy_ui_margin_height_px(&c_margin, c_tm, &parentsize))));
				cp_topleft.y += psy_ui_value_px(&c_margin.bottom, c_tm, &parentsize);
				cp_topleft.y += (int)height;				
			} else if (curr->align == psy_ui_ALIGN_VCLIENT) {
				const psy_ui_TextMetric* c_tm;
				psy_ui_RealRectangle position;
				psy_ui_Size componentsize;
				psy_ui_Size limit;

				position = psy_ui_component_position(curr);
				limit = psy_ui_component_scroll_size(group);
				c_tm = tm;
				componentsize = psy_ui_component_preferred_size(curr,
					&limit);
				cp_topleft.y += psy_ui_value_px(&c_margin.top, c_tm, NULL);
				psy_ui_component_setposition(curr,
					psy_ui_rectangle_make(
						psy_ui_point_make_px(
							position.left + psy_ui_value_px(&c_margin.left, c_tm, NULL),
							cp_topleft.y),
						psy_ui_size_make_px(
							(int)(psy_ui_value_px(&componentsize.width, c_tm, NULL)) -
								psy_ui_margin_width_px(&c_margin, c_tm, NULL),
							(int)height)));
				cp_topleft.y += psy_ui_value_px(&c_margin.bottom, c_tm, NULL);
				cp_topleft.y += (int)height;				
			} else if (curr->align == psy_ui_ALIGN_HCLIENT) {
				const psy_ui_TextMetric* c_tm;
				psy_ui_RealRectangle position;
				psy_ui_Size componentsize;
				psy_ui_Size limit;

				position = psy_ui_component_position(curr);
				limit = psy_ui_component_scroll_size(group);
				c_tm = tm;
				componentsize = psy_ui_component_preferred_size(curr,
					&limit);
				cp_topleft.y += psy_ui_value_px(&c_margin.top, c_tm, NULL);
				psy_ui_component_setposition(curr,
					psy_ui_rectangle_make(
						psy_ui_point_make_px(
							cp_topleft.x,
							position.top + psy_ui_value_px(
								&c_margin.top, c_tm, NULL)),
						psy_ui_size_make_px(
							(int)width,
							(int)(
								psy_ui_value_px(&componentsize.height, c_tm, NULL)) -
								psy_ui_margin_height_px(&c_margin, c_tm, NULL))));
				cp_topleft.x += (int)width;
				cp_topleft.y += psy_ui_value_px(&c_margin.bottom, c_tm, NULL);				
			}
		}
	}
}

void psy_ui_lclaligner_preferred_size(psy_ui_LCLAligner* self,
	psy_ui_Component* group, const psy_ui_Size* limit, psy_ui_Size* rv)
{				
	psy_ui_Size size;
	const psy_ui_TextMetric* tm;
	psy_ui_Component* client;
	psy_ui_Margin margin;
	psy_List* p;
	psy_List* q;
	psy_ui_RealPoint cp;
	psy_ui_Size maxsize;
	psy_ui_RealPoint cp_topleft;
	psy_ui_RealPoint cp_bottomright;

	assert(rv);
	size = *rv;		
	client = NULL;
	tm = psy_ui_component_textmetric(group);
	margin = psy_ui_component_margin(group);
	psy_ui_realpoint_init(&cp);
	psy_ui_size_init(&maxsize);			
	psy_ui_realpoint_init(&cp_topleft);
	psy_ui_realpoint_init(&cp_bottomright);			
	size.width = (!limit || (self->aligner.alignexpandmode &
		psy_ui_HEXPAND) == psy_ui_HEXPAND)
		? psy_ui_value_make_px(0)
		: limit->width;	
	if (limit && !((self->aligner.alignexpandmode &
			psy_ui_HEXPAND) == psy_ui_HEXPAND)) {
		psy_ui_aligner_adjust_spacing(group, &cp_topleft, &cp_bottomright);
		size.width = psy_ui_value_make_px(psy_ui_value_px(
			&limit->width, tm, NULL) - cp_topleft.x - cp_bottomright.x);
	}
	for (p = q = psy_ui_component_children(group, 0);
			p != NULL; p = p->next) {
		psy_ui_Component* curr;
					
		curr = (psy_ui_Component*)psy_list_entry(p);
		if (psy_ui_component_visible(curr)) {
			psy_ui_Size componentsize;
			psy_ui_Size limit;
			const psy_ui_TextMetric* c_tm;
			psy_ui_Margin c_margin;					

			c_margin = psy_ui_component_margin(curr);
			limit.width = psy_ui_value_make_px(psy_ui_value_px(
				&size.width, tm, NULL) - cp_topleft.x - cp_bottomright.x);
			limit.height = size.height;
			componentsize = psy_ui_component_preferred_size(curr, &limit);
			psy_ui_aligner_adjust_minmax_size(curr, tm, &componentsize,
				NULL);
			c_tm = psy_ui_component_textmetric(curr);
			if (curr->align == psy_ui_ALIGN_FIXED) {
				psy_ui_RealRectangle position;

				position = psy_ui_component_position(curr);
				if (psy_ui_value_px(&maxsize.height, tm, NULL) <
					position.top +
					psy_ui_value_px(&componentsize.height, c_tm, NULL) +
					psy_ui_margin_height_px(&c_margin, c_tm, NULL))
				{
					maxsize.height = psy_ui_value_make_px(position.top +
						psy_ui_value_px(&componentsize.height,
							c_tm, NULL) +
						psy_ui_margin_height_px(&c_margin,
							c_tm, NULL));
				}
				if (psy_ui_value_px(&maxsize.width, tm, NULL) <
						position.left +
						psy_ui_value_px(&componentsize.width, c_tm, NULL) +
						psy_ui_margin_width_px(&c_margin, c_tm, NULL)) {
					maxsize.width = psy_ui_value_make_px(
						position.left +
						psy_ui_value_px(&componentsize.width, c_tm, NULL) +
						psy_ui_margin_width_px(&c_margin, c_tm, NULL));
				}
			} else if (curr->align == psy_ui_ALIGN_CLIENT ||
					curr->align == psy_ui_ALIGN_CENTER) {
				client = curr;				
			} else if (curr->align == psy_ui_ALIGN_TOP ||
					curr->align == psy_ui_ALIGN_BOTTOM) {
				cp.y += psy_ui_value_px(&componentsize.height, c_tm, NULL) +
					psy_ui_margin_height_px(&c_margin, c_tm, NULL);
				if (psy_ui_value_px(&maxsize.height, c_tm, NULL) < cp.y) {
					maxsize.height = psy_ui_value_make_px(cp.y);
				}
				if (psy_ui_value_px(&maxsize.width, tm, NULL) <
						psy_ui_value_px(&componentsize.width, c_tm, NULL) +
						psy_ui_margin_width_px(&c_margin, c_tm, NULL)) {
					maxsize.width = psy_ui_value_make_px(
						psy_ui_value_px(&componentsize.width, c_tm, NULL) +
						psy_ui_margin_width_px(&c_margin, c_tm, NULL));
				}
			} else if (curr->align == psy_ui_ALIGN_RIGHT) {
				cp.x += psy_ui_value_px(&componentsize.width, c_tm, NULL) +
					psy_ui_margin_width_px(&c_margin, c_tm, NULL);
				cp_bottomright.x += (intptr_t)(psy_ui_value_px(
					&componentsize.width, c_tm, NULL) +
					psy_ui_margin_width_px(&c_margin, c_tm, NULL));
				if (psy_ui_value_px(&maxsize.width, c_tm, NULL) < cp.x) {
					maxsize.width = psy_ui_value_make_px(cp.x);
				}
				if (psy_ui_value_px(&maxsize.height, c_tm, NULL) < cp.y +
						psy_ui_value_px(&componentsize.height, c_tm, NULL) +
						psy_ui_margin_height_px(&c_margin, c_tm, NULL)) {
					maxsize.height = psy_ui_value_make_px(
						cp.y + psy_ui_value_px(&componentsize.height,
							c_tm, NULL) +
						psy_ui_margin_height_px(&c_margin, c_tm, NULL));
				}												
			} else if (curr->align == psy_ui_ALIGN_LEFT) {
				if (psy_ui_value_px(&size.width, tm, NULL) != 0) {
					double requiredcomponentwidth;

					requiredcomponentwidth = psy_ui_value_px(
						&componentsize.width, c_tm, &size) +
						psy_ui_margin_width_px(&c_margin, tm, &size);
					if (cp.x + requiredcomponentwidth >
							psy_ui_value_px(&size.width, c_tm, &size)) {
						cp.y = psy_ui_value_px(&maxsize.height, tm, &size);
						cp.x = 0;
						cp_topleft.x = 0;	
						componentsize = psy_ui_component_preferred_size(curr,
							&size);
					}						
				}						
				cp.x += psy_ui_value_px(&componentsize.width, c_tm, &size) +
					psy_ui_margin_width_px(&c_margin, c_tm, &size);
				cp_topleft.x += (intptr_t)psy_ui_value_px(&componentsize.width,
					c_tm, NULL) +
					psy_ui_margin_width_px(&c_margin, c_tm, &size);
				if (psy_ui_value_px(&maxsize.width, tm, &size) < cp.x) {
					maxsize.width = psy_ui_value_make_px((double)cp.x);
				}
				if (psy_ui_value_px(&maxsize.height, tm, &size) < cp.y +
						psy_ui_value_px(&componentsize.height, c_tm, &size) +
						psy_ui_margin_height_px(&c_margin, c_tm, &size)) {
					maxsize.height = psy_ui_value_make_px(cp.y +
						psy_ui_value_px(&componentsize.height, c_tm, &size) +
						psy_ui_margin_height_px(&c_margin, c_tm, &size));
				}
			}				
		}
	}
	psy_list_free(q);
	q = NULL;
	if (client) {
		psy_ui_Size componentsize;
		psy_ui_Size limit;
		const psy_ui_TextMetric* c_tm;
		psy_ui_Margin c_margin;					

		c_margin = psy_ui_component_margin(client);	
		if (size.width.quantity != 0) {
			limit.width = psy_ui_value_make_px(psy_ui_value_px(
				&size.width, tm, NULL) - cp_topleft.x - cp_bottomright.x);		
		} else {
			limit.width = psy_ui_value_make_px(0);
		}
		limit.height = size.height;		
		componentsize = psy_ui_component_preferred_size(client, &limit);
		psy_ui_aligner_adjust_minmax_size(client, tm, &componentsize,
			NULL);
		c_tm = psy_ui_component_textmetric(client);
		if (psy_ui_value_px(&maxsize.height, tm, NULL) < cp.y +
					psy_ui_value_px(&componentsize.height, c_tm, NULL) +
				psy_ui_margin_height_px(&c_margin, c_tm, NULL)) {
			maxsize.height = psy_ui_value_make_px(cp.y +
					psy_ui_value_px(&componentsize.height,
					c_tm, NULL) +
				psy_ui_margin_height_px(&c_margin,
					c_tm, NULL));
		}
		if (psy_ui_value_px(&maxsize.width, tm, NULL) <
				psy_ui_value_px(&componentsize.width, c_tm, NULL) +
			psy_ui_margin_width_px(&c_margin, c_tm, NULL)) {
			maxsize.width = psy_ui_value_make_px(
				psy_ui_value_px(&componentsize.width, c_tm, NULL) +
				psy_ui_margin_width_px(&c_margin, c_tm, NULL));
		}
	}
	*rv = maxsize;	
	psy_ui_aligner_add_border(group, rv);
}
