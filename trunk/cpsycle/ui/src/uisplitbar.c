// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uisplitbar.h"
// local
#include "uiapp.h"
// platform
#include "../../detail/trace.h"
#include "../../detail/portable.h"

// prototypes
static void splitbar_setalign(psy_ui_SplitBar*, psy_ui_AlignType);
static void splitbar_ondraw(psy_ui_SplitBar*, psy_ui_Graphics*);
static void splitbar_onmousedown(psy_ui_SplitBar*, psy_ui_MouseEvent*);
static void splitbar_onmousemove(psy_ui_SplitBar*, psy_ui_MouseEvent*);
static void splitbar_onmouseup(psy_ui_SplitBar*, psy_ui_MouseEvent*);
static void splitbar_onmouseenter(psy_ui_SplitBar*);
static psy_ui_Component* splitbar_prev(psy_ui_SplitBar*);
static psy_ui_Component* splitbar_next(psy_ui_SplitBar*);
static void splitbar_setcursor(psy_ui_SplitBar*);
static psy_ui_RealPoint splitbar_vthumbcenter(const psy_ui_SplitBar*);
static psy_ui_RealPoint splitbar_hthumbcenter(const psy_ui_SplitBar*);
// vtable
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_SplitBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super_vtable = *(self->component.vtable);
		vtable.setalign =
			(psy_ui_fp_component_setalign)
			splitbar_setalign;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			splitbar_ondraw;
		vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			splitbar_onmousedown;
		vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			splitbar_onmousemove;
		vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			splitbar_onmouseup;
		vtable.onmouseenter =
			(psy_ui_fp_component_onmouseenter)
			splitbar_onmouseenter;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}
// implementation
void psy_ui_splitbar_init(psy_ui_SplitBar* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_doublebuffer(&self->component);	
	self->resize = 0;
	self->hasrestore = FALSE;
	self->thumbsize = 30.0;
	self->isvertical = TRUE;
	psy_ui_size_init(&self->restoresize);
	psy_ui_component_setalign(&self->component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setstyletypes(&self->component,
		psy_ui_STYLE_SPLITTER, psy_ui_STYLE_SPLITTER_HOVER,
		psy_ui_STYLE_SPLITTER_SELECT, psy_INDEX_INVALID);	
}

void splitbar_ondraw(psy_ui_SplitBar* self, psy_ui_Graphics* g)
{		
	if (psy_ui_splitbar_isvertical(self)) {
		psy_ui_RealPoint center;

		center = splitbar_vthumbcenter(self);
		psy_ui_drawline(g, center, psy_ui_realpoint_make(center.x,
			center.y + self->thumbsize));
	} else {
		psy_ui_RealPoint center;

		center = splitbar_hthumbcenter(self);
		psy_ui_drawline(g, center,
			psy_ui_realpoint_make(center.x + self->thumbsize, center.y));
	}
}

psy_ui_RealPoint splitbar_vthumbcenter(const psy_ui_SplitBar* self)
{	
	psy_ui_RealSize size;

	size = psy_ui_component_size_px(&self->component);
	return psy_ui_realpoint_make(size.width / 2,
		size.height / 2 - self->thumbsize / 2);
}

psy_ui_RealPoint splitbar_hthumbcenter(const psy_ui_SplitBar* self)
{
	psy_ui_RealSize size;

	size = psy_ui_component_size_px(&self->component);
	return psy_ui_realpoint_make(size.width / 2 - self->thumbsize / 2,
		size.height / 2);
}

void splitbar_onmousedown(psy_ui_SplitBar* self, psy_ui_MouseEvent* ev)
{	
	if (ev->button == 2) {		
		psy_ui_Component* prev;

		prev = splitbar_prev(self);
		if (prev) {
			if (prev->align == psy_ui_ALIGN_LEFT ||
					prev->align == psy_ui_ALIGN_RIGHT) {
				if (!self->hasrestore) {
					self->restoresize = psy_ui_component_scrollsize(prev);
					self->hasrestore = TRUE;
					psy_ui_component_setpreferredsize(prev,
						psy_ui_size_make(psy_ui_value_make_ew(0),
							psy_ui_component_scrollsize(prev).height));
				} else {
					self->hasrestore = FALSE;
					psy_ui_component_setpreferredsize(prev,
						psy_ui_size_make(self->restoresize.width,
						psy_ui_component_scrollsize(prev).height));
				}
			} else
			if (prev->align == psy_ui_ALIGN_TOP ||
					prev->align == psy_ui_ALIGN_BOTTOM) {
				if (!self->hasrestore) {
					self->hasrestore = TRUE;
					self->restoresize = psy_ui_component_scrollsize(prev);					
					psy_ui_component_setpreferredsize(prev,
						psy_ui_size_make(
							psy_ui_component_scrollsize(prev).width,
							psy_ui_value_make_px(0)));
				} else {
					self->hasrestore = FALSE;
					psy_ui_component_setpreferredsize(prev,
						psy_ui_size_make(
							psy_ui_component_scrollsize(prev).width,
							self->restoresize.height));
				}
			}
			psy_ui_component_align(psy_ui_component_parent(&self->component));
			psy_ui_component_invalidate(psy_ui_component_parent(
				&self->component));
		}
	} else {
		psy_ui_RealRectangle position;

		position = psy_ui_component_position(&self->component);
		psy_ui_component_capture(&self->component);
		self->resize = 1;
		if (self->component.align == psy_ui_ALIGN_LEFT ||
			self->component.align == psy_ui_ALIGN_RIGHT) {
			self->dragoffset = ev->pt.x;
		} else {
			self->dragoffset = ev->pt.y;
		}		
		splitbar_setcursor(self);
	}
	if (self->resize) {
		psy_ui_component_addstylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
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
		prev = splitbar_prev(self);
		if (prev) {
			prevposition = psy_ui_component_position(prev);
		} else {
			prevposition = psy_ui_component_position(
				psy_ui_component_parent(&self->component));
		}
		next = splitbar_next(self);
		if (next) {
			nextposition = psy_ui_component_position(next);
		} else {
			nextposition = psy_ui_component_position(
				psy_ui_component_parent(&self->component));
		}
		switch (self->component.align) {
			case psy_ui_ALIGN_LEFT:
				psy_ui_component_move(&self->component,
					psy_ui_point_make_px(psy_max(prevposition.left, psy_min(
							nextposition.right - (position.right - position.left),
							position.left + ev->pt.x - self->dragoffset)),
						position.top));
				break;
			case psy_ui_ALIGN_RIGHT:				
				psy_ui_component_move(&self->component,
					psy_ui_point_make_px(
							psy_max(nextposition.left,
								psy_min(prevposition.right -
										psy_ui_realrectangle_width(&position),
									position.left + ev->pt.x -
										self->dragoffset)),
						position.top));
				break;
			case psy_ui_ALIGN_TOP:
				psy_ui_component_move(&self->component,
					psy_ui_point_make_px(
						position.left, psy_max(prevposition.top, psy_min(
							nextposition.bottom - (position.bottom - position.top),
						position.top + ev->pt.y - self->dragoffset))));
				break;
			case psy_ui_ALIGN_BOTTOM:				
				psy_ui_component_move(&self->component,
					psy_ui_point_make_px(
						position.left,
						psy_max(0.0, psy_min(prevposition.bottom -
							psy_ui_realrectangle_height(&position),
							position.top + ev->pt.y - self->dragoffset))));
				break;
			default:
				break;
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
		prev = splitbar_prev(self);
		next = splitbar_next(self);
		if (prev) {
			psy_signal_preventall(&prev->signal_hide);
			psy_ui_component_hide(prev);
			psy_signal_enableall(&prev->signal_hide);
			prev->visible = 1;
		}
		if (next) {
			psy_signal_preventall(&next->signal_hide);
			psy_ui_component_hide(next);
			psy_signal_enableall(&next->signal_hide);
			next->visible = 1;
		}
		if (prev) {
			position = psy_ui_component_position(&self->component);
			if (prev->align == psy_ui_ALIGN_LEFT) {
				psy_ui_RealRectangle prev_position;

				prev_position = psy_ui_component_position(prev);
				psy_ui_component_setpreferredsize(prev,
					psy_ui_size_make(
						psy_ui_value_make_px(position.left -
							prev_position.left),
						psy_ui_component_scrollsize(prev).height));
			} else if (prev->align == psy_ui_ALIGN_RIGHT) {
				psy_ui_RealRectangle prev_position;

				prev_position = psy_ui_component_position(prev);
				psy_ui_component_setpreferredsize(prev,
					psy_ui_size_make(
						psy_ui_value_make_px(
							prev_position.right - position.right),
						psy_ui_component_scrollsize(prev).height));
			} else if (prev->align == psy_ui_ALIGN_TOP) {				
				psy_ui_component_setpreferredsize(prev,
					psy_ui_size_make(
						psy_ui_component_scrollsize(prev).width,
						psy_ui_value_make_px(position.top)));
			} else if (prev->align == psy_ui_ALIGN_BOTTOM) {
				psy_ui_RealRectangle prev_position;
				
				prev_position = psy_ui_component_position(prev);				
				psy_ui_component_setpreferredsize(prev,
					psy_ui_size_make(
						psy_ui_component_scrollsize(prev).width,
						psy_ui_value_make_px(prev_position.bottom -
							position.bottom)));
			}
			psy_ui_component_align(psy_ui_component_parent(&self->component));
			psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
		}
		if (prev) {
			psy_ui_component_show(prev);
		}
		if (next) {
			psy_ui_component_show(next);
		}
		splitbar_setcursor(self);		
		if (prev) {
			psy_ui_component_invalidate(prev);
		}
		if (next) {
			psy_ui_component_invalidate(next);
		}
		psy_ui_component_invalidate(&self->component);
	}	
	self->resize = 0;
	psy_ui_component_removestylestate(&self->component,
		psy_ui_STYLESTATE_SELECT);	
}

void splitbar_onmouseenter(psy_ui_SplitBar* self)
{		
	super_vtable.onmouseenter(psy_ui_splitbar_base(self));
	splitbar_setcursor(self);	
}

psy_ui_Component* splitbar_prev(psy_ui_SplitBar* self)
{
	psy_ui_Component* rv;
	psy_List* c;

	rv = NULL;
	c = psy_ui_component_children(psy_ui_component_parent(&self->component),
			psy_ui_NONRECURSIVE);
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

psy_ui_Component* splitbar_next(psy_ui_SplitBar* self)
{
	psy_ui_Component* rv;
	psy_List* c;

	rv = NULL;
	c = psy_ui_component_children(psy_ui_component_parent(&self->component),
			psy_ui_NONRECURSIVE);
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
	psy_ui_component_setcursor(&self->component,
		(psy_ui_splitbar_isvertical(self))
		? psy_ui_CURSORSTYLE_COL_RESIZE
		: psy_ui_CURSORSTYLE_ROW_RESIZE);
}

void splitbar_setalign(psy_ui_SplitBar* self, psy_ui_AlignType align)
{	
	self->isvertical = (self->component.align == psy_ui_ALIGN_LEFT ||
		self->component.align == psy_ui_ALIGN_RIGHT);
	psy_ui_component_setpreferredsize(&self->component,
		(psy_ui_splitbar_isvertical(self))
		? psy_ui_size_make_em(1.3, 1.5)
		: psy_ui_size_make_em(1.5, 0.5));
}
