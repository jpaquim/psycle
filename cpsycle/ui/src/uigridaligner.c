/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uigridaligner.h"
/* ui */
#include "uicomponent.h"
/* std */
#include <math.h>
#include <stdio.h>

/* prototypes */
static void psy_ui_gridaligner_align(psy_ui_GridAligner*);
static void psy_ui_gridaligner_preferredsize(psy_ui_GridAligner*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static uintptr_t psy_ui_gridaligner_numclients(psy_ui_GridAligner*);
/* vtable */
static psy_ui_AlignerVTable gridaligner_vtable;
static bool gridaligner_vtable_initialized = FALSE;

static void gridaligner_vtable_init(psy_ui_GridAligner* self)
{
	if (!gridaligner_vtable_initialized) {
		gridaligner_vtable = *(self->aligner.vtable);		
		gridaligner_vtable.align =
			(psy_ui_fp_aligner_align)
			psy_ui_gridaligner_align;
		gridaligner_vtable.preferredsize =
			(psy_ui_fp_aligner_preferredsize)
			psy_ui_gridaligner_preferredsize;
		gridaligner_vtable_initialized = TRUE;
	}
	self->aligner.vtable = &gridaligner_vtable;
}
/* implementation */
void psy_ui_gridaligner_init(psy_ui_GridAligner* self, psy_ui_Component* component)
{
	psy_ui_aligner_init(&self->aligner);
	gridaligner_vtable_init(self);
	self->component = component;
	self->numcols = 3;
}

void psy_ui_gridaligner_align(psy_ui_GridAligner* self)
{	
	psy_ui_Size size;
	psy_ui_Size parentsize;
	const psy_ui_TextMetric* tm;
	psy_ui_RealPoint cp_topleft;
	psy_ui_RealPoint cp_bottomright;
	double cpymax = 0;
	psy_ui_RealSize colsize;
	psy_List* p;
	psy_List* q;
	psy_List* wrap = 0;	
	uintptr_t currcol = 0;
	
	assert(self->component);

	size = psy_ui_component_scrollsize(self->component);	
	if (psy_ui_component_parent_const(self->component)) {
		parentsize = psy_ui_component_scrollsize(
			psy_ui_component_parent_const(self->component));
	} else {
		parentsize = psy_ui_component_scrollsize(self->component);
	}
	tm = psy_ui_component_textmetric(self->component);
	psy_ui_realpoint_init(&cp_topleft);
	psy_ui_realpoint_init_all(&cp_bottomright,
		psy_ui_value_px(&size.width, tm, &parentsize),
		psy_ui_value_px(&size.height, tm, &parentsize));
	psy_ui_aligner_adjustborder(self->component, &cp_topleft, &cp_bottomright);
	psy_ui_aligner_adjustspacing(self->component, &cp_topleft, &cp_bottomright);
	colsize.width = (cp_bottomright.x - cp_topleft.x) / (double) self->numcols;
	colsize.height = cp_bottomright.y - cp_topleft.y;
	for (p = q = psy_ui_component_children(self->component, 0); p != NULL;
			psy_list_next(&p)) {
		psy_ui_Component* component;
			
		component = (psy_ui_Component*)psy_list_entry(p);
		if (psy_ui_component_visible(component)) {
			psy_ui_Size componentsize;			
			psy_ui_Size limit;
			psy_ui_Margin c_margin;			
			const psy_ui_TextMetric* c_tm;
				
			psy_ui_size_init_px(&limit,
				cp_bottomright.x - cp_topleft.x,
				cp_bottomright.y - cp_topleft.y);						
			componentsize = psy_ui_component_preferredsize(component,
					&limit);			
			psy_ui_aligner_adjustminmaxsize(component, tm, &componentsize,
				NULL); /* todo percent */
			componentsize.width = psy_ui_value_make_px(colsize.width);
			c_tm = psy_ui_component_textmetric(component);
			c_margin = psy_ui_component_margin(component);			
			cp_topleft.x += psy_ui_value_px(&c_margin.left, c_tm, &size);
			psy_ui_component_setposition(component,
				psy_ui_rectangle_make(
					psy_ui_point_make(
						psy_ui_value_make_px(cp_topleft.x),
						psy_ui_value_make_px(cp_topleft.y +
							psy_ui_value_px(&c_margin.top, c_tm, &size))),
					psy_ui_size_make(
						componentsize.width,
						psy_ui_value_make_px(
							cp_bottomright.y - cp_topleft.y -
							psy_ui_margin_height_px(&c_margin, c_tm, &size)))));
			cp_topleft.x += psy_ui_value_px(&c_margin.right, c_tm, &size);
			cp_topleft.x += psy_ui_value_px(&componentsize.width, c_tm, &size);
			if (cpymax < cp_topleft.y +
				psy_ui_value_px(&componentsize.height, c_tm, &size) +
				psy_ui_margin_height_px(&c_margin, c_tm, &size)) {
				cpymax = cp_topleft.y + psy_ui_value_px(
					&componentsize.height, c_tm, &size) +
					psy_ui_margin_height_px(&c_margin, c_tm, &size);
			}		
			++currcol;
			if (currcol == self->numcols) {
				currcol = 0;
				cp_topleft.x = 0;
				cp_topleft.y = cpymax;
			}
		}
	}	
	psy_list_free(q);
	psy_list_free(wrap);	
}

void psy_ui_gridaligner_preferredsize(psy_ui_GridAligner* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{			
	if (rv) {
		psy_ui_Size size;
		const psy_ui_TextMetric* tm;
		psy_ui_Component* client;
		psy_ui_Margin margin;
		psy_ui_Size colsize;
				
		size = *rv;
		client = NULL;
		tm = psy_ui_component_textmetric(self->component);
		margin = psy_ui_component_margin(self->component);
		if (self->component->containeralign &&
				self->component->containeralign->containeralign != psy_ui_CONTAINER_ALIGN_NONE) {
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
			size.width = (!limit || (self->component->containeralign->alignexpandmode &
				psy_ui_HEXPAND) == psy_ui_HEXPAND)
				? psy_ui_value_make_px(0)
				: limit->width;
			colsize.width = psy_ui_mul_value_real(size.width, 1.0 / (double)self->numcols);
			colsize.height = size.height;
			for (p = q = psy_ui_component_children(self->component, 0);
					p != NULL; p = p->next) {
				psy_ui_Component* component;
					
				component = (psy_ui_Component*)psy_list_entry(p);				
				if (psy_ui_component_visible(component)) {
					psy_ui_Size componentsize;
					psy_ui_Size limit;
					const psy_ui_TextMetric* c_tm;
					psy_ui_Margin c_margin;					

					c_margin = psy_ui_component_margin(component);					
					limit.width = psy_ui_value_make_px(psy_ui_value_px(
						&size.width, tm, NULL) - cp_topleft.x - cp_bottomright.x);
					limit.height = size.height;
					componentsize = psy_ui_component_preferredsize(component,
						&limit);					
					psy_ui_aligner_adjustminmaxsize(component, tm,
						&componentsize, NULL); /* todo percent */
					c_tm = psy_ui_component_textmetric(component);	
					cp.x += psy_ui_value_px(&colsize.width, c_tm, NULL) +
						psy_ui_margin_width_px(&c_margin, c_tm, NULL);
					cp_topleft.x += (intptr_t)psy_ui_value_px(&colsize.width,
						c_tm, NULL) +
						psy_ui_margin_width_px(&c_margin, c_tm, NULL);
					if (psy_ui_value_px(&maxsize.width, tm, NULL) < cp.x) {
						maxsize.width = psy_ui_value_make_px((double)cp.x);
					}
					if (psy_ui_value_px(&maxsize.height, tm, NULL) < cp.y +
						psy_ui_value_px(&componentsize.height, c_tm, NULL) +
						psy_ui_margin_height_px(&c_margin, c_tm, NULL)) {
						maxsize.height = psy_ui_value_make_px(cp.y +
							psy_ui_value_px(&componentsize.height, c_tm, NULL) +
							psy_ui_margin_height_px(&c_margin, c_tm, NULL));
					}
				}
			}
			psy_list_free(q);
			*rv = maxsize;			
		} else {
			*rv = size;			
		}
		psy_ui_aligner_addspacingandborder(self->component, rv);
	}	
}
