// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uisplitbar.h"

static void splitbar_onmousedown(ui_splitbar*, ui_component* sender, MouseEvent*);
static void splitbar_onmousemove(ui_splitbar*, ui_component* sender, MouseEvent*);
static void splitbar_onmouseup(ui_splitbar*, ui_component* sender, MouseEvent*);
static void splitbar_onmouseenter(ui_splitbar*, ui_component* sender);
static void splitbar_onmouseleave(ui_splitbar*, ui_component* sender);
static void splitbar_onpreferredsize(ui_splitbar*, ui_component* sender,
	ui_size* limit, ui_size* size);
static ui_component* splitbar_prevcomponent(ui_splitbar*);
static void splitbar_setcursor(ui_splitbar*);

void ui_splitbar_init(ui_splitbar* self, ui_component* parent)
{		
	self->resize = 0;
	ui_component_init(&self->component, parent);
	ui_component_setalign(&self->component, UI_ALIGN_LEFT);
	ui_component_resize(&self->component, 4, 5);
	psy_signal_connect(&self->component.signal_mousedown, self, splitbar_onmousedown);
		psy_signal_connect(&self->component.signal_mousemove, self, splitbar_onmousemove);
	psy_signal_connect(&self->component.signal_mouseup, self, splitbar_onmouseup);
	psy_signal_connect(&self->component.signal_mouseenter, self, splitbar_onmouseenter);
	psy_signal_connect(&self->component.signal_mouseleave, self, splitbar_onmouseleave);	
	psy_signal_disconnectall(&self->component.signal_preferredsize);
	psy_signal_connect(&self->component.signal_preferredsize, self,
		splitbar_onpreferredsize);
}

void splitbar_onpreferredsize(ui_splitbar* self, ui_component* sender,
	ui_size* limit, ui_size* rv)
{		
	if (rv) {		
		ui_textmetric tm;		

		tm = ui_component_textmetric(&self->component);
		
		rv->width = (int)(tm.tmAveCharWidth * 0.8);
		rv->height = (int) (tm.tmHeight * 0.5);
	} else {
		*rv = ui_component_size(&self->component);
	}
}

void splitbar_onmousedown(ui_splitbar* self, ui_component* sender, MouseEvent* ev)
{	
	ui_component_capture(sender);
	self->resize = 1;
	splitbar_setcursor(self);
}

void splitbar_onmousemove(ui_splitbar* self, ui_component* sender, MouseEvent* ev)
{
	if (self->resize == 1) {		
		ui_rectangle position;
			
		position = ui_component_position(sender);
		if (sender->align == UI_ALIGN_LEFT) {
			ui_component_move(sender, position.left + ev->x, position.top);
		} else {
			ui_component_move(sender, position.left, position.top + ev->y);
		}
		ui_component_invalidate(sender);
		ui_component_update(sender);		
	} else {
		ui_component_invalidate(sender);
	}
	splitbar_setcursor(self);
}

void splitbar_onmouseup(ui_splitbar* self, ui_component* sender, MouseEvent* ev)
{			
	ui_rectangle position;
	ui_component* prev;

	ui_component_releasecapture();
	self->resize = 0;
	prev = splitbar_prevcomponent(self);
	if (prev) {		
		position = ui_component_position(sender);
		if (prev->align == UI_ALIGN_LEFT) {
			ui_component_resize(prev, position.left,
				ui_component_size(prev).height);
		} else
		if (prev->align == UI_ALIGN_TOP) {
			ui_component_resize(prev, ui_component_size(prev).width,
				position.top);
		}
		ui_component_align(ui_component_parent(sender));
	}
	splitbar_setcursor(self);
}

void splitbar_onmouseenter(ui_splitbar* self, ui_component* sender)
{	
	ui_component_setbackgroundcolor(sender, 0x00666666);
	ui_component_invalidate(sender);	
	splitbar_setcursor(self);
}

void splitbar_onmouseleave(ui_splitbar* self, ui_component* sender)
{			
	ui_component_setbackgroundcolor(sender, 0x00232323);
	ui_component_invalidate(sender);
}

ui_component* splitbar_prevcomponent(ui_splitbar* self)
{
	ui_component* rv = 0;
	psy_List* c;

	c = ui_component_children(ui_component_parent(&self->component), 0);	
	while (c) {
		if (c->entry == &self->component) {
			c = c->prev;
			while (c && ((ui_component*)c->entry)->align != 
					self->component.align) {
				c = c->prev;
			}
			break;
		}
		c = c->next;
	}	
	if (c) {		
		rv = (ui_component*) c->entry;
	}
	psy_list_free(c);
	return rv;
}

void splitbar_setcursor(ui_splitbar* self)
{
	if (self->component.align == UI_ALIGN_LEFT) {
		SetCursor(LoadCursor(NULL, IDC_SIZEWE));
	} else
	if (self->component.align == UI_ALIGN_TOP) {
		SetCursor(LoadCursor(NULL, IDC_SIZENS));
	}
}