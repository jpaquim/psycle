// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uisplitbar.h"
#include "uiapp.h"

#include "../../detail/trace.h"
#include "../../detail/portable.h"

static void splitbar_ondraw(psy_ui_SplitBar*, psy_ui_Graphics*);
static void splitbar_onmousedown(psy_ui_SplitBar*, psy_ui_MouseEvent*);
static void splitbar_onmousemove(psy_ui_SplitBar*, psy_ui_MouseEvent*);
static void splitbar_onmouseup(psy_ui_SplitBar*, psy_ui_MouseEvent*);
static void splitbar_onmouseenter(psy_ui_SplitBar*);
static void splitbar_onmouseleave(psy_ui_SplitBar*);
static psy_ui_Component* splitbar_prevcomponent(psy_ui_SplitBar*);
static psy_ui_Component* splitbar_nextcomponent(psy_ui_SplitBar*);
static void splitbar_setcursor(psy_ui_SplitBar*);
static void splitbar_onpreferredsize(psy_ui_SplitBar*, psy_ui_Size* limit,
	psy_ui_Size* rv);

static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_SplitBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_component_ondraw)splitbar_ondraw;
		vtable.onmousedown = (psy_ui_fp_component_onmousedown)splitbar_onmousedown;
		vtable.onmousemove = (psy_ui_fp_component_onmousemove)splitbar_onmousemove;
		vtable.onmouseup = (psy_ui_fp_component_onmouseup)splitbar_onmouseup;
		vtable.onmouseenter = (psy_ui_fp_component_onmouseenter)splitbar_onmouseenter;
		vtable.onmouseleave = (psy_ui_fp_component_onmouseleave)splitbar_onmouseleave;
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			splitbar_onpreferredsize;
		vtable_initialized = TRUE;
	}
}

void psy_ui_splitbar_init(psy_ui_SplitBar* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	psy_ui_component_doublebuffer(&self->component);
	self->component.vtable = &vtable;	
	self->hover = 0;
	self->resize = 0;
	self->hasrestore = FALSE;
	psy_ui_size_init(&self->restoresize);
	psy_ui_component_setalign(&self->component, psy_ui_ALIGN_LEFT);
}

void splitbar_ondraw(psy_ui_SplitBar* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;	
	double center;
	double thumbsize;
	double ident;
	
	size = psy_ui_component_sizepx(&self->component);
	thumbsize = 30;
	psy_ui_setcolour(g, psy_ui_colour_make(0x00303030));
	if (self->component.align == psy_ui_ALIGN_LEFT ||
			self->component.align == psy_ui_ALIGN_RIGHT) {
		center = size.height / 2 - thumbsize / 2;
		ident = size.width / 2;		
		psy_ui_drawline(g, ident, center, ident, center + thumbsize);
	} else {
		center = size.width / 2 - thumbsize / 2;
		ident = size.height / 2;		
		psy_ui_drawline(g, center, ident, center + thumbsize, ident);
	}
}

void splitbar_onmousedown(psy_ui_SplitBar* self, psy_ui_MouseEvent* ev)
{	
	if (ev->button == 2) {		
		psy_ui_Component* prev;

		prev = splitbar_prevcomponent(self);
		if (prev) {
			if (prev->align == psy_ui_ALIGN_LEFT ||
					prev->align == psy_ui_ALIGN_RIGHT) {
				if (!self->hasrestore) {
					self->restoresize = psy_ui_component_size(prev);
					self->hasrestore = TRUE;
					psy_ui_component_resize(prev,
						psy_ui_size_make(psy_ui_value_makepx(0),
							psy_ui_component_size(prev).height));
				} else {
					self->hasrestore = FALSE;
					psy_ui_component_resize(prev,
						psy_ui_size_make(self->restoresize.width,
						psy_ui_component_size(prev).height));
				}
			} else
			if (prev->align == psy_ui_ALIGN_TOP ||
					prev->align == psy_ui_ALIGN_BOTTOM) {
				if (!self->hasrestore) {
					self->hasrestore = TRUE;
					self->restoresize = psy_ui_component_size(prev);					
					psy_ui_component_resize(prev,
						psy_ui_size_make(
							psy_ui_component_size(prev).width,
							psy_ui_value_makepx(0)));
				} else {
					self->hasrestore = FALSE;
					psy_ui_component_resize(prev,
						psy_ui_size_make(
							psy_ui_component_size(prev).width,
							self->restoresize.height));
				}
			}
			psy_ui_component_align(psy_ui_component_parent(&self->component));
		}
	} else {
		psy_ui_RealRectangle position;

		position = psy_ui_component_position(&self->component);
		psy_ui_component_capture(&self->component);
		self->resize = 1;
		if (self->component.align == psy_ui_ALIGN_LEFT ||
			self->component.align == psy_ui_ALIGN_RIGHT) {
			self->dragoffset = ev->x;
		} else {
			self->dragoffset = ev->y;
		}		
		splitbar_setcursor(self);
	}
	if (self->resize) {
		psy_ui_component_setbackgroundcolour(&self->component,
			psy_ui_colour_make(0x00333333));
		psy_ui_component_invalidate(&self->component);
	}
}

void splitbar_onmousemove(psy_ui_SplitBar* self, psy_ui_MouseEvent* ev)
{
	if (self->resize == 1) {		
		psy_ui_RealRectangle position;
		psy_ui_RealRectangle parentposition;
		psy_ui_RealRectangle prevposition;
		psy_ui_RealRectangle nextposition;
		psy_ui_Component* parent;
		psy_ui_Component* prev;
		psy_ui_Component* next;
			
		position = psy_ui_component_position(&self->component);
		parent = psy_ui_component_parent(&self->component);
		if (parent) {
			parentposition = psy_ui_component_position(parent);
		} else {
			parentposition = position;
		}
		prev = splitbar_prevcomponent(self);
		if (prev) {
			prevposition = psy_ui_component_position(prev);
		} else {
			prevposition = psy_ui_component_position(psy_ui_component_parent(&self->component));
		}
		next = splitbar_nextcomponent(self);
		if (next) {
			nextposition = psy_ui_component_position(next);
		} else {
			nextposition = psy_ui_component_position(psy_ui_component_parent(&self->component));
		}
		if (self->component.align == psy_ui_ALIGN_LEFT) {
			psy_ui_component_move(&self->component,
				psy_ui_point_make(
					psy_ui_value_makepx(
						psy_max(prevposition.left,
						psy_min(nextposition.right - (position.right - position.left), position.left + ev->x - self->dragoffset))),
					psy_ui_value_makepx(position.top)));
		} else
		if (self->component.align == psy_ui_ALIGN_RIGHT) {
			psy_ui_component_move(&self->component,
				psy_ui_point_make(
					psy_ui_value_makepx(
						psy_max(nextposition.left,
						psy_min(prevposition.right - (position.right - position.left), position.left + ev->x - self->dragoffset))),
					psy_ui_value_makepx(position.top)));
		} else
		if (self->component.align == psy_ui_ALIGN_TOP) {
			psy_ui_component_move(&self->component,
				psy_ui_point_make(
					psy_ui_value_makepx(position.left),
					psy_ui_value_makepx(
						psy_max(prevposition.top,
						psy_min(nextposition.bottom - (position.bottom - position.top), position.top + ev->y - self->dragoffset)))
					));
		} else
		if (self->component.align == psy_ui_ALIGN_BOTTOM) {
			psy_ui_component_move(&self->component,
				psy_ui_point_make(
					psy_ui_value_makepx(position.left),
					psy_ui_value_makepx(
						psy_max(0,
						psy_min((parentposition.bottom - parentposition.top) - (position.bottom - position.top),
							 position.top + ev->y - self->dragoffset)))));
			
		}			
		psy_ui_component_invalidate(&self->component);
		psy_ui_component_update(&self->component);
	} else {
		psy_ui_component_invalidate(&self->component);
	}
	splitbar_setcursor(self);
}

void splitbar_onmouseup(psy_ui_SplitBar* self, psy_ui_MouseEvent* ev)
{			
	if (ev->button == 1) {
		psy_ui_RealRectangle position;
		psy_ui_Component* prev;
		psy_ui_Component* next;

		psy_ui_component_releasecapture(&self->component);
		prev = splitbar_prevcomponent(self);
		next = splitbar_nextcomponent(self);
		if (prev) {
			psy_ui_component_hide(prev);
			prev->visible = 1;
		}
		if (next) {
			psy_ui_component_hide(next);
			next->visible = 1;
		}
		if (prev) {
			position = psy_ui_component_position(&self->component);
			if (prev->align == psy_ui_ALIGN_LEFT) {
				psy_ui_RealRectangle prev_position;

				prev->preventpreferredsizeatalign = TRUE;
				prev_position = psy_ui_component_position(prev);
				psy_ui_component_resize(prev,
					psy_ui_size_make(
						psy_ui_value_makepx(position.left - prev_position.left),
						psy_ui_component_size(prev).height));
			} else if (prev->align == psy_ui_ALIGN_RIGHT) {
					psy_ui_RealRectangle prev_position;

				prev->preventpreferredsizeatalign = TRUE;
				prev_position = psy_ui_component_position(prev);
				psy_ui_component_resize(prev,
					psy_ui_size_make(
						psy_ui_value_makepx(prev_position.right - position.right),
						psy_ui_component_size(prev).height));
			} else if (prev->align == psy_ui_ALIGN_TOP) {
				prev->preventpreferredsizeatalign = TRUE;
				psy_ui_component_resize(prev,
					psy_ui_size_make(
						psy_ui_component_size(prev).width,
						psy_ui_value_makepx(position.top)));
			} else if (prev->align == psy_ui_ALIGN_BOTTOM) {
				psy_ui_RealRectangle prev_position;

				prev->preventpreferredsizeatalign = TRUE;
				prev_position = psy_ui_component_position(prev);
				psy_ui_component_resize(prev,
					psy_ui_size_make(
						psy_ui_component_size(prev).width,
						psy_ui_value_makepx(prev_position.bottom - position.bottom)));
			}
			psy_ui_component_align(psy_ui_component_parent(&self->component));					
		}
		if (prev) {
			psy_ui_component_show(prev);
		}
		if (next) {
			psy_ui_component_show(next);
		}
		splitbar_setcursor(self);		
		self->hover = 1;
	}
	if (self->resize) {
		psy_ui_component_setbackgroundcolour(&self->component,
			psy_ui_app()->defaults.style_common.backgroundcolour);
		psy_ui_component_invalidate(&self->component);
		self->resize = 0;
	}
}

void splitbar_onmouseenter(psy_ui_SplitBar* self)
{		
	splitbar_setcursor(self);
	self->hover = 1;	
}

void splitbar_onmouseleave(psy_ui_SplitBar* self)
{			
	self->hover = 0;
}

psy_ui_Component* splitbar_prevcomponent(psy_ui_SplitBar* self)
{
	psy_ui_Component* rv = 0;
	psy_List* c;

	c = psy_ui_component_children(psy_ui_component_parent(&self->component),
		0);
	while (c) {
		if (c->entry == &self->component) {
			c = c->prev;
			while (c && (((psy_ui_Component*)c->entry)->align != 
					self->component.align ||
					!psy_ui_component_visible((psy_ui_Component*)c->entry))) {
				c = c->prev;
			}
			break;
		}
		c = c->next;
	}	
	if (c && psy_ui_component_visible((psy_ui_Component*)c->entry)) {
		rv = (psy_ui_Component*) c->entry;
	}
	psy_list_free(c);
	return rv;
}

psy_ui_Component* splitbar_nextcomponent(psy_ui_SplitBar* self)
{
	psy_ui_Component* rv = 0;
	psy_List* c;

	c = psy_ui_component_children(psy_ui_component_parent(&self->component),
		0);
	while (c) {
		if (c->entry == &self->component) {
			c = c->next;
			while (c && (((psy_ui_Component*)c->entry)->align !=
				self->component.align ||
				!psy_ui_component_visible((psy_ui_Component*)c->entry))) {
				c = c->next;
			}
			break;
		}
		c = c->next;
	}
	if (c && psy_ui_component_visible((psy_ui_Component*)c->entry)) {
		rv = (psy_ui_Component*)c->entry;
	}
	psy_list_free(c);
	return rv;
}

void splitbar_setcursor(psy_ui_SplitBar* self)
{
	if (self->component.align == psy_ui_ALIGN_LEFT ||
			self->component.align == psy_ui_ALIGN_RIGHT) {
		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
	} else
	if (self->component.align == psy_ui_ALIGN_TOP ||
			self->component.align == psy_ui_ALIGN_BOTTOM) {
		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_ROW_RESIZE);
	}
}

void splitbar_onpreferredsize(psy_ui_SplitBar* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (self->component.align == psy_ui_ALIGN_LEFT ||
			self->component.align == psy_ui_ALIGN_RIGHT) {
		*rv = psy_ui_size_make(psy_ui_value_makeew(1.2),
				psy_ui_value_makeeh(1.5));
	} else {
		*rv = psy_ui_size_make(psy_ui_value_makeew(1.5),
			psy_ui_value_makeeh(0.5));
	}
}
