// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uialigner.h"

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
		
	if (self->component->debugflag == 65) {
		self = self;
	}
	size = psy_ui_component_size(self->component);
	tm = psy_ui_component_textmetric(self->component);
	cp_bottomright.x = size.width;
	cp_bottomright.y = size.height;
	for (p = q = psy_ui_component_children(self->component, 0); p != 0;
			p = p->next) {
		psy_ui_Component* component;
			
		component = (psy_ui_Component*)p->entry;		
		if (component->visible) {
			psy_ui_Size componentsize;
			psy_ui_Size limit;

			limit.width = cp_bottomright.x - cp_topleft.x;
			limit.height = cp_bottomright.y - cp_topleft.y;			
			componentsize = psy_ui_component_preferredsize(component, &limit);
			if (component->align == psy_ui_ALIGN_CLIENT) {
				client = component;
			} else
			if (component->align == psy_ui_ALIGN_FILL) {
				psy_ui_component_setposition(component,
					psy_ui_value_px(&component->margin.left, &tm),
					psy_ui_value_px(&component->margin.top, &tm),				
					size.width - psy_ui_value_px(&component->margin.left, &tm)
						- psy_ui_value_px(&component->margin.right, &tm),
					size.height -
						psy_ui_margin_height_px(&component->margin, &tm));
			} else
			if (component->align == psy_ui_ALIGN_TOP) {
				cp_topleft.y += psy_ui_value_px(&component->margin.top, &tm);
				psy_ui_component_setposition(component, 
					cp_topleft.x + psy_ui_value_px(&component->margin.left, &tm),
					cp_topleft.y,
					cp_bottomright.x - cp_topleft.x -
						psy_ui_margin_width_px(&component->margin, &tm),
					componentsize.height);
				cp_topleft.y += psy_ui_value_px(&component->margin.bottom, &tm);
				cp_topleft.y += componentsize.height;
			} else
			if (component->align == psy_ui_ALIGN_BOTTOM) {
				cp_bottomright.y -=
					psy_ui_value_px(&component->margin.bottom, &tm);
				psy_ui_component_setposition(component, 
					cp_topleft.x + psy_ui_value_px(&component->margin.left, &tm), 
					cp_bottomright.y - componentsize.height,
					cp_bottomright.x - cp_topleft.x -
						psy_ui_margin_width_px(&component->margin, &tm),
					componentsize.height);
				cp_bottomright.y -= psy_ui_value_px(&component->margin.top, &tm);
				cp_bottomright.y -= componentsize.height;
			} else
			if (component->align == psy_ui_ALIGN_RIGHT) {
				int requiredcomponentwidth;

				requiredcomponentwidth = componentsize.width +
					psy_ui_margin_width_px(&component->margin, &tm);
				cp_bottomright.x -= requiredcomponentwidth;
				psy_ui_component_setposition(component,
					cp_bottomright.x + psy_ui_value_px(&component->margin.left, &tm),
					cp_topleft.y +
						psy_ui_value_px(&component->margin.top, &tm),
					componentsize.width,										
					cp_bottomright.y - cp_topleft.y - 
						psy_ui_margin_height_px(&component->margin, &tm));
			} else
			if (component->align == psy_ui_ALIGN_LEFT) {				
				if ((self->component->alignexpandmode & psy_ui_HORIZONTALEXPAND)
						== psy_ui_HORIZONTALEXPAND) {
				} else {
					int requiredcomponentwidth;

					requiredcomponentwidth = componentsize.width +
						psy_ui_margin_width_px(&component->margin, &tm);
					if (cp_topleft.x + requiredcomponentwidth > size.width) {
						psy_List* w;						
						cp_topleft.x = 0;
						for (w = wrap; w != 0; w = w->next) {
							psy_ui_Component* c;
							c = (psy_ui_Component*)w->entry;
							psy_ui_component_resize(c, psy_ui_component_size(c).width,
								cpymax - cp_topleft.y -
								psy_ui_margin_height_px(&component->margin, &tm));
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
					component->justify == psy_ui_JUSTIFY_EXPAND 
					? cp_bottomright.y - cp_topleft.y - 
						psy_ui_margin_height_px(&component->margin, &tm)
					: componentsize.height);
				cp_topleft.x += psy_ui_value_px(&component->margin.right, &tm);
				cp_topleft.x += componentsize.width;				
				if (cpymax < cp_topleft.y + componentsize.height +
						psy_ui_margin_height_px(&component->margin, &tm)) {
					cpymax = cp_topleft.y + componentsize.height +
						psy_ui_margin_height_px(&component->margin, &tm);
				}
			}				
		}
	}
	if (client) {		
		psy_ui_component_setposition(client,
			cp_topleft.x + psy_ui_value_px(&client->margin.left, &tm),
			cp_topleft.y + psy_ui_value_px(&client->margin.top, &tm),
			cp_bottomright.x - cp_topleft.x -
				psy_ui_margin_width_px(&client->margin, &tm),
			cp_bottomright.y - cp_topleft.y -
				psy_ui_margin_height_px(&client->margin, &tm));
	}
	psy_list_free(q);
	psy_list_free(wrap);	
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
				? 0
				: limit->width;
			for (p = q = psy_ui_component_children(self->component, 0); p != 0;
					p = p->next) {
				psy_ui_Component* component;
					
				component = (psy_ui_Component*)p->entry;		
				if (component->visible) {
					psy_ui_Size componentsize;
					psy_ui_Size limit;

					limit.width = size.width - cp_topleft.x -
						cp_bottomright.x;
					limit.height = size.height;
					componentsize = psy_ui_component_preferredsize(component, &limit);
					if (component->align == psy_ui_ALIGN_CLIENT) {
						if (maxsize.height < cp.y + componentsize.height +
							psy_ui_margin_height_px(&component->margin, &tm)) {
							maxsize.height = cp.y + componentsize.height +
								psy_ui_margin_height_px(&component->margin, &tm);
						}
						if (maxsize.width < componentsize.width +
							psy_ui_margin_width_px(&component->margin, &tm)) {
							maxsize.width = componentsize.width +
								psy_ui_margin_width_px(&component->margin, &tm);
						}
					} else
					if (component->align == psy_ui_ALIGN_TOP ||
							component->align == psy_ui_ALIGN_BOTTOM) {
						cp.y += componentsize.height +
							psy_ui_margin_height_px(&component->margin, &tm);
						if (maxsize.height < cp.y) {
							maxsize.height = cp.y;
						}
						if (maxsize.width < componentsize.width +
								psy_ui_margin_width_px(&component->margin, &tm)) {
							maxsize.width = componentsize.width +
								psy_ui_margin_width_px(&component->margin, &tm);
						}
					} else
					if (component->align == psy_ui_ALIGN_RIGHT) {
						cp.x += componentsize.width +
							psy_ui_margin_width_px(&component->margin, &tm);
						cp_bottomright.x += componentsize.width +
							psy_ui_margin_width_px(&component->margin, &tm);
						if (maxsize.width < cp.x) {
							maxsize.width = cp.x;
						}
						if (maxsize.height < cp.y + componentsize.height +
								psy_ui_margin_height_px(&component->margin, &tm)) {
							maxsize.height = cp.y + componentsize.height +
								psy_ui_margin_height_px(&component->margin, &tm);
						}												
					} else
					if (component->align == psy_ui_ALIGN_LEFT) {
						if (size.width != 0) {
							int requiredcomponentwidth;

							requiredcomponentwidth = componentsize.width +
								psy_ui_margin_width_px(&component->margin, &tm);
							if (cp.x + requiredcomponentwidth > size.width) {
								cp.y = maxsize.height;
								cp.x = 0;
								cp_topleft.x = 0;
							}						
						}						
						cp.x += componentsize.width +
							psy_ui_margin_width_px(&component->margin, &tm);
						cp_topleft.x += componentsize.width +
							psy_ui_margin_width_px(&component->margin, &tm);
						if (maxsize.width < cp.x) {
							maxsize.width = cp.x;
						}
						if (maxsize.height < cp.y + componentsize.height +
								psy_ui_margin_height_px(&component->margin, &tm)) {
							maxsize.height = cp.y + componentsize.height +
								psy_ui_margin_height_px(&component->margin, &tm);
						}
					}				
				}
			}
			psy_list_free(q);
			*rv = maxsize;
			rv->width += psy_ui_margin_width_px(&self->component->spacing, &tm);
			rv->height += psy_ui_margin_height_px(&self->component->spacing, &tm);			
		} else {
			*rv = size;
			rv->width += psy_ui_margin_width_px(&self->component->spacing, &tm);
			rv->height += psy_ui_margin_height_px(&self->component->spacing, &tm);
		}
	}	
}
