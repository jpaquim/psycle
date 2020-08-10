// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiscroller.h"

#include <stdlib.h>
#include <string.h>

#include "../../detail/portable.h"
#include "../../detail/trace.h"

static void psy_ui_scroller_onsize(psy_ui_Scroller*, psy_ui_Component* sender, psy_ui_Size*);
static void psy_ui_scroller_onscroll(psy_ui_Scroller*, psy_ui_Component* sender);
static void psy_ui_scroller_horizontal_onchanged(psy_ui_Scroller*, psy_ui_ScrollBar* sender);
static void psy_ui_scroller_vertical_onchanged(psy_ui_Scroller*, psy_ui_ScrollBar* sender);
static void psy_ui_scroller_scrollrangechanged(psy_ui_Scroller*, psy_ui_Component* sender,
	psy_ui_Orientation);
static void psy_ui_scroller_connectclient(psy_ui_Scroller*);

/*void psy_ui_scroller_init(psy_ui_Scroller* self, psy_ui_Component* client,
	psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	// horizontal scrollbar
	psy_ui_scrollbar_init(&self->bottom, &self->component);
	psy_ui_scrollbar_setorientation(&self->bottom, psy_ui_HORIZONTAL);
	psy_ui_component_setalign(&self->bottom.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(&self->bottom.component);
	// vertical scrollbar
	psy_ui_scrollbar_init(&self->right, &self->component);
	psy_ui_scrollbar_setorientation(&self->right, psy_ui_VERTICAL);
	psy_ui_component_setalign(&self->right.component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(&self->right.component);
	// reparent client
	self->client = client;
	//psy_ui_component_setparent(client, &self->component);
	//psy_ui_component_setalign(client, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->right.signal_changed, self,
		psy_ui_scroller_vertical_onchanged);
	psy_signal_connect(&self->bottom.signal_changed, self,
		psy_ui_scroller_horizontal_onchanged);
	psy_ui_scrollbar_setscrollrange(&self->right, 0, 100);
}*/

void psy_ui_scroller_init(psy_ui_Scroller* self, psy_ui_Component* client,
	psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	// horizontal scrollbar
	psy_ui_scrollbar_init(&self->bottom, &self->component);
	psy_ui_scrollbar_setorientation(&self->bottom, psy_ui_HORIZONTAL);
	psy_ui_component_setalign(&self->bottom.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_hide(&self->bottom.component);
	// vertical scrollbar
	psy_ui_scrollbar_init(&self->right, &self->component);
	psy_ui_scrollbar_setorientation(&self->right, psy_ui_VERTICAL);
	psy_ui_component_setalign(&self->right.component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_hide(&self->right.component);
	// reparent client
	self->client = client;
	psy_ui_component_setparent(client, &self->component);
	psy_ui_component_setalign(client, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->right.signal_changed, self,
		psy_ui_scroller_vertical_onchanged);
	psy_signal_connect(&self->bottom.signal_changed, self,
		psy_ui_scroller_horizontal_onchanged);
	psy_ui_scrollbar_setscrollrange(&self->right, 0, 100);
	psy_ui_scroller_connectclient(self);
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

/*
void psy_ui_scroller_updateoverflow(psy_ui_Scroller* self)
{
	if ((self->client->overflow & psy_ui_OVERFLOW_VSCROLL) == psy_ui_OVERFLOW_VSCROLL) {
		psy_ui_Size preferredsize;
		psy_ui_TextMetric tm;
		int maxlines;
		int visilines;
		int currline;
		psy_ui_Size size;

		tm = psy_ui_component_textmetric(self->client);
		size = psy_ui_component_size(self->client);
		preferredsize = psy_ui_component_preferredsize(self->client, &size);
		maxlines = (int)(psy_ui_value_px(&preferredsize.height, &tm) / (double)self->client->scrollstepy);
		visilines = (psy_ui_value_px(&size.height, &tm) / self->client->scrollstepy);
		currline = psy_ui_component_scrolltop(self->client) / self->client->scrollstepy;
		if ((self->client->overflow & psy_ui_OVERFLOW_VSCROLLCENTER) ==
			psy_ui_OVERFLOW_VSCROLLCENTER) {
			intptr_t minscroll;
			intptr_t maxscroll;

			psy_ui_scrollbar_setscrollrange(&self->right, -visilines / 2,
				maxlines - visilines / 2 - 1);
			psy_ui_component_setverticaluserscrollrange(self->client, -visilines / 2,
				maxlines - visilines / 2 - 1);
			if (currline > maxlines - visilines / 2) {
				currline = max(-visilines / 2, maxlines - visilines / 2);
				psy_ui_component_setscrolltop(self->client, currline * self->client->scrollstepy);
			}
			psy_ui_component_verticaluserscrollrange(self->client, &minscroll, &maxscroll);
			if (maxscroll - minscroll <= 0) {
				if (psy_ui_component_visible(&self->right.component)) {
					psy_ui_component_hide(&self->right.component);
					psy_ui_component_align(&self->component);
				}
			} else {
				if (!psy_ui_component_visible(&self->right.component)) {
					psy_ui_component_show(&self->right.component);
					psy_ui_component_align(&self->component);
				}
			}
		} else {
			intptr_t minscroll;
			intptr_t maxscroll;

			psy_ui_scrollbar_setscrollrange(&self->right, 0, maxlines - visilines);
			psy_ui_component_setverticaluserscrollrange(self->client, 0, maxlines - visilines);
			if (currline > maxlines - visilines) {
				currline = max(0, maxlines - visilines);
				psy_ui_component_setscrolltop(self->client, currline * self->client->scrollstepy);
			}
			//psy_ui_scrollbar_scrollrange(&self->right, &minscroll, &maxscroll);
			psy_ui_component_verticaluserscrollrange(self->client, &minscroll, &maxscroll);
			if (maxscroll - minscroll <= 0) {
				if (psy_ui_component_visible(&self->right.component)) {
					psy_ui_component_hide(&self->right.component);
					psy_ui_component_align(&self->component);
				}
			} else {
				if (!psy_ui_component_visible(&self->right.component)) {
					psy_ui_component_show(&self->right.component);
					psy_ui_component_align(&self->component);
				}
			}
		}
	}
	if ((self->client->overflow & psy_ui_OVERFLOW_HSCROLL) == psy_ui_OVERFLOW_HSCROLL) {
		psy_ui_Size preferredsize;
		psy_ui_TextMetric tm;
		int maxrows;
		int visirows;
		int currrow;
		psy_ui_Size size;
		intptr_t minscroll;
		intptr_t maxscroll;

		tm = psy_ui_component_textmetric(self->client);
		size = psy_ui_component_size(self->client);
		preferredsize = psy_ui_component_preferredsize(self->client, &size);
		maxrows = (int)(psy_ui_value_px(&preferredsize.width, &tm) /
			(double)self->client->scrollstepx + 0.5);
		visirows = psy_ui_value_px(&size.width, &tm) / self->client->scrollstepx;
		currrow = psy_ui_component_scrollleft(self->client) / self->client->scrollstepx;
		psy_ui_scrollbar_setscrollrange(&self->bottom, 0, maxrows - visirows);
		if (currrow > maxrows - visirows) {
			currrow = max(0, maxrows - visirows);
			psy_ui_component_setscrollleft(self->client, currrow * self->client->scrollstepx);
		}
		psy_ui_scrollbar_scrollrange(&self->bottom, &minscroll, &maxscroll);
		if (maxscroll - minscroll <= 0) {
			if (psy_ui_component_visible(&self->bottom.component)) {
				psy_ui_component_hide(&self->bottom.component);
				psy_ui_component_align(&self->component);
			}
		} else {
			if (!psy_ui_component_visible(&self->bottom.component)) {
				psy_ui_component_show(&self->bottom.component);
				psy_ui_component_align(&self->component);
			}
		}
	}
}*/

void psy_ui_scroller_horizontal_onchanged(psy_ui_Scroller* self, psy_ui_ScrollBar* sender)
{
	assert(self->client);
	int iPos;
	int nPos;

	iPos = self->client->scroll.x / self->client->scrollstepx;
	nPos = psy_ui_scrollbar_position(sender);
	psy_ui_component_setscrollleft(self->client,
		psy_ui_component_scrollleft(self->client) -
		self->client->scrollstepx * (iPos - nPos));
}

void psy_ui_scroller_vertical_onchanged(psy_ui_Scroller* self, psy_ui_ScrollBar* sender)
{
	assert(self->client);
	int iPos;
	int nPos;

	iPos = self->client->scroll.y / self->client->scrollstepy;
	nPos = psy_ui_scrollbar_position(sender);
	psy_ui_component_setscrolltop(self->client,
		psy_ui_component_scrolltop(self->client) -
		self->client->scrollstepy * (iPos - nPos));
}

void psy_ui_scroller_onscroll(psy_ui_Scroller* self, psy_ui_Component* sender)
{
	int nPos;

	nPos = self->client->scroll.y / self->client->scrollstepy;
	psy_ui_scrollbar_setthumbposition(&self->right, nPos);
	nPos = self->client->scroll.x / self->client->scrollstepx;
	psy_ui_scrollbar_setthumbposition(&self->bottom, nPos);
}

void psy_ui_scroller_scrollrangechanged(psy_ui_Scroller* self, psy_ui_Component* sender,
	psy_ui_Orientation orientation)
{
	if (orientation == psy_ui_VERTICAL) {
		psy_ui_scrollbar_setscrollrange(&self->right, sender->vscrollrange.x,
			sender->vscrollrange.y);
		if (sender->vscrollrange.y - sender->vscrollrange.x <= 0) {
			if (psy_ui_component_visible(&self->right.component)) {
				psy_ui_component_hide(&self->right.component);
				psy_ui_component_align(&self->component);
			}
		} else {
			if (!psy_ui_component_visible(&self->right.component)) {
				psy_ui_component_show(&self->right.component);
				psy_ui_component_align(&self->component);
			}
		}
	} else if (orientation == psy_ui_HORIZONTAL) {
		psy_ui_scrollbar_setscrollrange(&self->bottom, sender->hscrollrange.x,
			sender->hscrollrange.y);
		if (sender->hscrollrange.y - sender->hscrollrange.x <= 0) {
			if (psy_ui_component_visible(&self->bottom.component)) {
				psy_ui_component_hide(&self->bottom.component);
				psy_ui_component_align(&self->component);
			}
		} else {
			if (!psy_ui_component_visible(&self->bottom.component)) {
				psy_ui_component_show(&self->bottom.component);
				psy_ui_component_align(&self->component);
			}
		}
	}
}
