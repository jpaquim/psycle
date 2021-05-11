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
static void splitter_setalign(psy_ui_Splitter*, psy_ui_AlignType);
static void splitter_ondraw(psy_ui_Splitter*, psy_ui_Graphics*);
static void splitter_onmousedown(psy_ui_Splitter*, psy_ui_MouseEvent*);
static void splitter_onmousemove(psy_ui_Splitter*, psy_ui_MouseEvent*);
static void splitter_onmouseup(psy_ui_Splitter*, psy_ui_MouseEvent*);
static void splitter_onmouseenter(psy_ui_Splitter*);
static void splitter_onhide(psy_ui_Splitter*);
static void splitter_onshow(psy_ui_Splitter*);
static psy_ui_Component* splitter_prev(psy_ui_Splitter*);
static psy_ui_Component* splitter_next(psy_ui_Splitter*);
static void splitter_setcursor(psy_ui_Splitter*);
static psy_ui_RealPoint splitter_vthumbcenter(const psy_ui_Splitter*);
static psy_ui_RealPoint splitter_hthumbcenter(const psy_ui_Splitter*);
static void psy_ui_splitter_onbutton(psy_ui_Splitter*);
static void psy_ui_splitter_onshowtoggle(psy_ui_Splitter*, psy_ui_Component* sender);
static void psy_ui_splitter_onhidetoggle(psy_ui_Splitter*, psy_ui_Component* sender);
// vtable
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_Splitter* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super = *(self->component.vtable);
		vtable.setalign =
			(psy_ui_fp_component_setalign)
			splitter_setalign;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			splitter_ondraw;
		vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			splitter_onmousedown;
		vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			splitter_onmousemove;
		vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			splitter_onmouseup;
		vtable.onmouseenter =
			(psy_ui_fp_component_onmouseenter)
			splitter_onmouseenter;
		vtable.hide =
			(psy_ui_fp_component_hide)
			splitter_onhide;
		vtable.show =
			(psy_ui_fp_component_show)
			splitter_onshow;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}
// implementation
void psy_ui_splitter_init(psy_ui_Splitter* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_doublebuffer(&self->component);	
	self->resize = 0;
	self->hasrestore = FALSE;
	self->thumbsize = 30.0;
	self->isvertical = TRUE;
	self->toggle = NULL;
	self->button = NULL;
	psy_ui_size_init(&self->restoresize);
	psy_ui_component_setalign(&self->component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setstyletypes(&self->component,
		psy_ui_STYLE_SPLITTER, psy_ui_STYLE_SPLITTER_HOVER,
		psy_ui_STYLE_SPLITTER_SELECT, psy_INDEX_INVALID);	
}

void splitter_ondraw(psy_ui_Splitter* self, psy_ui_Graphics* g)
{		
	if (psy_ui_splitter_isvertical(self)) {
		psy_ui_RealPoint center;

		center = splitter_vthumbcenter(self);
		psy_ui_drawline(g, center, psy_ui_realpoint_make(center.x,
			center.y + self->thumbsize));
	} else {
		psy_ui_RealPoint center;

		center = splitter_hthumbcenter(self);
		psy_ui_drawline(g, center,
			psy_ui_realpoint_make(center.x + self->thumbsize, center.y));
	}
}

psy_ui_RealPoint splitter_vthumbcenter(const psy_ui_Splitter* self)
{	
	psy_ui_RealSize size;

	size = psy_ui_component_size_px(&self->component);
	return psy_ui_realpoint_make(size.width / 2,
		size.height / 2 - self->thumbsize / 2);
}

psy_ui_RealPoint splitter_hthumbcenter(const psy_ui_Splitter* self)
{
	psy_ui_RealSize size;

	size = psy_ui_component_size_px(&self->component);
	return psy_ui_realpoint_make(size.width / 2 - self->thumbsize / 2,
		size.height / 2);
}

void splitter_onmousedown(psy_ui_Splitter* self, psy_ui_MouseEvent* ev)
{	
	if (ev->button == 2) {		
		psy_ui_Component* prev;

		prev = splitter_prev(self);
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
		splitter_setcursor(self);
	}
	if (self->resize) {
		psy_ui_component_addstylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

void splitter_onmousemove(psy_ui_Splitter* self, psy_ui_MouseEvent* ev)
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
		prev = splitter_prev(self);
		if (prev) {
			prevposition = psy_ui_component_position(prev);
		} else {
			prevposition = psy_ui_component_position(
				psy_ui_component_parent(&self->component));
		}
		next = splitter_next(self);
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
	splitter_setcursor(self);
}

void splitter_onmouseup(psy_ui_Splitter* self, psy_ui_MouseEvent* ev)
{			
	if (ev->button == 1) {
		psy_ui_RealRectangle position;
		psy_ui_Component* prev;
		psy_ui_Component* next;

		psy_ui_component_releasecapture(&self->component);
		prev = splitter_prev(self);
		next = splitter_next(self);
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
		splitter_setcursor(self);		
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

void splitter_onmouseenter(psy_ui_Splitter* self)
{		
	super.onmouseenter(psy_ui_splitter_base(self));
	splitter_setcursor(self);	
}

psy_ui_Component* splitter_prev(psy_ui_Splitter* self)
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

psy_ui_Component* splitter_next(psy_ui_Splitter* self)
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

void splitter_setcursor(psy_ui_Splitter* self)
{	
	psy_ui_component_setcursor(&self->component,
		(psy_ui_splitter_isvertical(self))
		? psy_ui_CURSORSTYLE_COL_RESIZE
		: psy_ui_CURSORSTYLE_ROW_RESIZE);
}

void splitter_setalign(psy_ui_Splitter* self, psy_ui_AlignType align)
{	
	self->isvertical = (self->component.align == psy_ui_ALIGN_LEFT ||
		self->component.align == psy_ui_ALIGN_RIGHT);
	psy_ui_component_setpreferredsize(&self->component,
		(psy_ui_splitter_isvertical(self))
		? psy_ui_size_make_em(1.3, 1.5)
		: psy_ui_size_make_em(1.5, 0.5));
}

void splitter_onhide(psy_ui_Splitter* self)
{
	super.hide(&self->component);
	if (self->toggle && psy_ui_component_visible(self->toggle)) {
		psy_signal_prevent(&self->toggle->signal_hide, self,
			psy_ui_splitter_onhidetoggle);
		psy_ui_component_hide_align(self->toggle);
		psy_signal_enable(&self->toggle->signal_hide, self,
			psy_ui_splitter_onhidetoggle);
	}
	if (self->button) {
		psy_ui_button_disablehighlight(self->button);
	}
}

void splitter_onshow(psy_ui_Splitter* self)
{
	super.show(&self->component);
	if (self->toggle && !psy_ui_component_visible(self->toggle)) {
		psy_signal_prevent(&self->toggle->signal_show, self,
			psy_ui_splitter_onshowtoggle);
		psy_ui_component_show_align(self->toggle);
		psy_signal_enable(&self->toggle->signal_show, self,
			psy_ui_splitter_onshowtoggle);
	}
	if (self->button) {
		psy_ui_button_highlight(self->button);
	}
}

void psy_ui_splitter_settoggle(psy_ui_Splitter* self, psy_ui_Component* toggle)
{
	if (self->toggle) {
		psy_signal_disconnect(&self->toggle->signal_hide, self,
			psy_ui_splitter_onhidetoggle);
		psy_signal_disconnect(&self->toggle->signal_show, self,
			psy_ui_splitter_onshowtoggle);
	}
	self->toggle = toggle;
	psy_signal_connect(&self->toggle->signal_hide, self,
		psy_ui_splitter_onhidetoggle);
	psy_signal_connect(&self->toggle->signal_show, self,
		psy_ui_splitter_onshowtoggle);
}

void psy_ui_splitter_setbutton(psy_ui_Splitter* self, psy_ui_Button* button)
{	
	if (self->button) {
		psy_signal_disconnect(&self->button->signal_clicked, self,
			psy_ui_splitter_onbutton);
	}
	self->button = button;
	psy_signal_connect(&self->button->signal_clicked, self,
		psy_ui_splitter_onbutton);
}


void psy_ui_splitter_onbutton(psy_ui_Splitter* self)
{
	if (psy_ui_component_visible(&self->component)) {
		psy_ui_component_hide(&self->component);
	} else {
		psy_ui_component_show(&self->component);
	}
}

void psy_ui_splitter_onshowtoggle(psy_ui_Splitter* self, psy_ui_Component* sender)
{	
	psy_ui_component_show(&self->component);
}

void psy_ui_splitter_onhidetoggle(psy_ui_Splitter* self, psy_ui_Component* sender)
{
	psy_ui_component_hide(&self->component);
}
