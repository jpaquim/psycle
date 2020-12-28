// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyvscroll 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiscroller.h"

#include <stdlib.h>
#include <string.h>

#include "../../detail/portable.h"
#include "../../detail/trace.h"

static void psy_ui_scroller_onsize(psy_ui_Scroller*, psy_ui_Component* sender, psy_ui_Size*);
static void psy_ui_scroller_onscroll(psy_ui_Scroller*, psy_ui_Component* sender);
static void psy_ui_scroller_onscrollbarclicked(psy_ui_Scroller*, psy_ui_Component* sender);
static void psy_ui_scroller_horizontal_onchanged(psy_ui_Scroller*, psy_ui_ScrollBar* sender);
static void psy_ui_scroller_vertical_onchanged(psy_ui_Scroller*, psy_ui_ScrollBar* sender);
static void psy_ui_scroller_scrollrangechanged(psy_ui_Scroller*, psy_ui_Component* sender,
	psy_ui_Orientation);
static void psy_ui_scroller_connectclient(psy_ui_Scroller*);
static void psy_ui_scroller_onfocus(psy_ui_Scroller* self, psy_ui_Component* sender);

void psy_ui_scroller_init(psy_ui_Scroller* self, psy_ui_Component* client,
	psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_BACKGROUND_NONE);
	// bottom
	psy_ui_component_init(&self->bottom, &self->component);
	psy_ui_component_setalign(&self->bottom, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(&self->bottom);
	// spacer
	psy_ui_component_init(&self->spacer, &self->bottom);
	psy_ui_component_setalign(&self->spacer, psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(&self->spacer);
	psy_ui_component_setpreferredsize(&self->spacer,
		psy_ui_size_makeem(2.5, 1.0));
	psy_ui_component_preventalign(&self->spacer);
	// horizontal scrollbar
	psy_ui_scrollbar_init(&self->hscroll, &self->bottom);
	psy_ui_scrollbar_setorientation(&self->hscroll, psy_ui_HORIZONTAL);
	psy_ui_component_setalign(&self->hscroll.component, psy_ui_ALIGN_CLIENT);	
	psy_signal_connect(&self->hscroll.signal_clicked, self,
		psy_ui_scroller_onscrollbarclicked);
	// vertical scrollbar
	psy_ui_scrollbar_init(&self->vscroll, &self->component);
	psy_ui_scrollbar_setorientation(&self->vscroll, psy_ui_VERTICAL);
	psy_ui_component_setalign(&self->vscroll.component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(&self->vscroll.component);
	psy_signal_connect(&self->hscroll.signal_clicked, self,
		psy_ui_scroller_onscrollbarclicked);	
	// reparent client
	self->client = client;
	psy_ui_component_setparent(client, &self->component);
	psy_ui_component_setalign(client, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->vscroll.signal_changed, self,
		psy_ui_scroller_vertical_onchanged);
	psy_signal_connect(&self->hscroll.signal_changed, self,
		psy_ui_scroller_horizontal_onchanged);
	psy_ui_scrollbar_setscrollrange(&self->vscroll, 0, 100);
	psy_ui_scroller_connectclient(self);
	psy_signal_connect(&self->component.signal_focus, self,
		psy_ui_scroller_onfocus);
}

void psy_ui_scroller_connectclient(psy_ui_Scroller* self)
{
	psy_signal_connect(&self->client->signal_scrollrangechanged, self,
		psy_ui_scroller_scrollrangechanged);		
	psy_signal_connect(&self->client->signal_size, self,
		psy_ui_scroller_onsize);
	psy_signal_connect(&self->client->signal_scroll, self,
		psy_ui_scroller_onscroll);		
	psy_ui_component_setalign(self->client, psy_ui_ALIGN_CLIENT);
}

void psy_ui_scroller_onsize(psy_ui_Scroller* self, psy_ui_Component* sender, psy_ui_Size* size)
{
	//if (self->client->overflow != psy_ui_OVERFLOW_HIDDEN) {
		// psy_ui_scroller_updateoverflow(self);
	//}
}

void psy_ui_scroller_horizontal_onchanged(psy_ui_Scroller* self, psy_ui_ScrollBar* sender)
{
	assert(self->client);
	intptr_t iPos;
	intptr_t nPos;

	iPos = self->client->scroll.x / self->client->scrollstepx;
	nPos = psy_ui_scrollbar_position(sender);
	psy_ui_component_setscrollleft(self->client,
		psy_ui_component_scrollleft(self->client) -
		self->client->scrollstepx * (iPos - nPos));
}

void psy_ui_scroller_vertical_onchanged(psy_ui_Scroller* self, psy_ui_ScrollBar* sender)
{
	assert(self->client);
	intptr_t iPos;
	intptr_t nPos;

	iPos = self->client->scroll.y / self->client->scrollstepy;
	nPos = psy_ui_scrollbar_position(sender);
	psy_ui_component_setscrolltop(self->client,
		psy_ui_component_scrolltop(self->client) -
		self->client->scrollstepy * (iPos - nPos));
}

void psy_ui_scroller_onscroll(psy_ui_Scroller* self, psy_ui_Component* sender)
{
	intptr_t nPos;

	nPos = self->client->scroll.y / self->client->scrollstepy;
	psy_ui_scrollbar_setthumbposition(&self->vscroll, nPos);
	nPos = self->client->scroll.x / self->client->scrollstepx;
	psy_ui_scrollbar_setthumbposition(&self->hscroll, nPos);
}

void psy_ui_scroller_scrollrangechanged(psy_ui_Scroller* self, psy_ui_Component* sender,
	psy_ui_Orientation orientation)
{
	if (orientation == psy_ui_VERTICAL) {
		psy_ui_scrollbar_setscrollrange(&self->vscroll, sender->vscrollrange.x,
			sender->vscrollrange.y);
		if (sender->vscrollrange.y - sender->vscrollrange.x <= 0) {
			if (psy_ui_component_visible(&self->vscroll.component)) {
				psy_ui_component_hide(&self->vscroll.component);
				psy_ui_component_hide(&self->spacer);
				psy_ui_component_align(&self->component);
			}
		} else if (!psy_ui_component_visible(&self->vscroll.component)) {
			psy_ui_component_show(&self->vscroll.component);
			if (psy_ui_component_visible(&self->hscroll.component)) {
				psy_ui_component_show(&self->spacer);
			}
			psy_ui_component_align(&self->component);			
		}
	} else if (orientation == psy_ui_HORIZONTAL) {
		psy_ui_scrollbar_setscrollrange(&self->hscroll, sender->hscrollrange.x,
			sender->hscrollrange.y);
		if (sender->hscrollrange.y - sender->hscrollrange.x <= 0) {
			if (psy_ui_component_visible(&self->bottom)) {
				psy_ui_component_hide(&self->bottom);
				psy_ui_component_hide(&self->spacer);
				psy_ui_component_align(&self->component);
			}
		} else if (!psy_ui_component_visible(&self->bottom)) {
			psy_ui_component_show(&self->bottom);
			if (psy_ui_component_visible(&self->vscroll.component)) {
				psy_ui_component_show(&self->spacer);
			}
			psy_ui_component_align(&self->component);			
		}
	}
}

void psy_ui_scroller_onscrollbarclicked(psy_ui_Scroller* self, psy_ui_Component* sender)
{
	if (self->client) {
		psy_ui_component_setfocus(self->client);
	}
}

void psy_ui_scroller_onfocus(psy_ui_Scroller* self, psy_ui_Component* sender)
{
	if (self->client) {
		psy_ui_component_setfocus(self->client);
	}
}
