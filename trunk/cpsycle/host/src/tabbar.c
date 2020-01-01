// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "tabbar.h"
#include <string.h>

static void tabbar_ondraw(TabBar*, psy_ui_Component* sender, psy_ui_Graphics*);
static void tabbar_ondestroy(TabBar*, psy_ui_Component* component);
static void tabbar_onmousedown(TabBar*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void tabbar_onmousemove(TabBar*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void tabbar_onmouseenter(TabBar*, psy_ui_Component* sender);
static void tabbar_onmouseleave(TabBar*, psy_ui_Component* sender);
static int tabbar_tabhittest(TabBar* self, int x, int y);
static void tabbar_onalign(TabBar*, psy_ui_Component* sender);
static void tabbar_preferredsize(TabBar*, ui_size* limit, ui_size* rv);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(TabBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.preferredsize = (psy_ui_fp_preferredsize)
			tabbar_preferredsize;
	}
}

void tab_init(Tab* self, const char* text, ui_size* size)
{
	self->text = _strdup(text);
	if (size) {
		self->size = *size;
	}	
	ui_margin_init(&self->margin,
		ui_value_makepx(0),
		ui_value_makeew(0),
		ui_value_makepx(0),
		ui_value_makepx(0));
}

void tab_dispose(Tab* self)
{
	free(self->text);
}

void tabbar_init(TabBar* self, psy_ui_Component* parent)
{
	ui_component_init(&self->component, parent);
	self->tabs = 0;
	self->selected = 0;
	self->hover = 0;
	self->hoverindex = -1;
	self->tabalignment = UI_ALIGN_TOP;
	self->component.doublebuffered = 1;	
	psy_signal_init(&self->signal_change);
	psy_signal_connect(&self->component.signal_draw, self, tabbar_ondraw);
	psy_signal_connect(&self->component.signal_destroy, self,
		tabbar_ondestroy);
	psy_signal_connect(&self->component.signal_align, self, tabbar_onalign);	
	psy_signal_connect(&self->component.signal_mousedown, self,
		tabbar_onmousedown);
	psy_signal_connect(&self->component.signal_mousemove, self,
		tabbar_onmousemove);
	psy_signal_connect(&self->component.signal_mouseenter, self,
		tabbar_onmouseenter);
	psy_signal_connect(&self->component.signal_mouseleave, self,
		tabbar_onmouseleave);
	vtable_init(self);
	self->component.vtable = &vtable;

}

void tabbar_ondestroy(TabBar* self, psy_ui_Component* component)
{	
	psy_List* p;	

	for (p = self->tabs; p != 0; p = p->next) {	
		tab_dispose((Tab*)(p->entry));
		free(p->entry);
	}
	psy_list_free(self->tabs);
	psy_signal_dispose(&self->signal_change);
}

void tabbar_ondraw(TabBar* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{	
	psy_List* tabs;
	int c = 0;
	int cpx = 0;
	int cpy = 0;
	int cpxsel = 0;
	int selwidth = 0;	
	int cpxhover = 0;
	int hoverwidth = 0;
	ui_size size;
	ui_textmetric tm;
	
	size = ui_component_size(&self->component);
	tm = ui_component_textmetric(&self->component);
	ui_setbackgroundmode(g, TRANSPARENT);	
	if (self->tabalignment == UI_ALIGN_TOP) {
		cpx = 0;
	} else 
	if (self->tabalignment == UI_ALIGN_RIGHT) {
		cpy = 5;
		cpx = 10;
	} else
	if (self->tabalignment == UI_ALIGN_LEFT) {		
		cpy = 5;
	}
	for (tabs = self->tabs; tabs != 0; tabs = tabs->next, ++c) {
		Tab* tab;

		tab = (Tab*)tabs->entry;
		if (self->selected == c) {
			ui_settextcolor(g, 0x00B1C8B0);
			cpxsel = cpx + ui_value_px(&tab->margin.left, &tm);
			selwidth = tab->size.width;
		}
		if (self->hover && self->hoverindex == c && self->hoverindex != self->selected) {
			ui_settextcolor(g, 0x00FFFFFF);
			cpxhover = cpx + ui_value_px(&tab->margin.left, &tm);
			hoverwidth = tab->size.width;
		} else
		if (self->selected == c) {
			ui_settextcolor(g, 0x00B1C8B0);		
		} else {
			ui_settextcolor(g, 0x00D1C5B6);
		}		
		if (self->tabalignment == UI_ALIGN_TOP) {
			cpx += ui_value_px(&tab->margin.left, &tm);
			ui_textout(g, cpx, cpy, tab->text, strlen(tab->text));
			cpx += tab->size.width + ui_value_px(&tab->margin.right, &tm);
		} else
		if (self->tabalignment == UI_ALIGN_LEFT ||
			self->tabalignment == UI_ALIGN_RIGHT) {
			cpy += ui_value_px(&tab->margin.top, &tm);
			ui_textout(g, cpx, cpy, tab->text, strlen(tab->text));
			cpy += tab->size.height + ui_value_px(&tab->margin.bottom, &tm);
		}
	}
	ui_setcolor(g, 0x005F5F5F);		
	if (self->tabalignment == UI_ALIGN_TOP) {
		int y;

		ui_setcolor(g, 0x00B1C8B0);		
		y = tm.tmHeight + 2;
		ui_drawline(g, cpxsel, y, cpxsel + selwidth, y);		 
		if (self->hover && self->hoverindex != self->selected) {		
			ui_setcolor(g, 0x00FFFFFF);
			ui_drawline(g, cpxhover, y, cpxhover + hoverwidth, y);
		}
	} else
	if (self->tabalignment == UI_ALIGN_LEFT) {
		ui_drawline(g, size.width - 2, 0, size.width - 2, cpy);
	}
	if (self->tabalignment == UI_ALIGN_RIGHT) {
		ui_drawline(g, 0, 0, 0, cpy);
	}
}

void tabbar_onmousedown(TabBar* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	int tabindex;
	
	tabindex = tabbar_tabhittest(self, ev->x, ev->y);
	if (tabindex != -1 && tabindex != self->selected)  {
		self->selected = tabbar_tabhittest(self, ev->x, ev->y);
		ui_component_invalidate(&self->component);
		psy_signal_emit(&self->signal_change, self, 1, self->selected);
	}		
}

int tabbar_tabhittest(TabBar* self, int x, int y) 
{
	psy_List* tabs;	
	int rv = -1;
	int c = 0;
	int cpx = 0;
	int cpy = 0;
	ui_textmetric tm;

	tm = ui_component_textmetric(&self->component);
	if (self->tabalignment == UI_ALIGN_TOP) {
		cpx = 0;
	} else 
	if (self->tabalignment == UI_ALIGN_RIGHT) {
		cpy = 5;
		cpx = 10;
	} else
	if (self->tabalignment == UI_ALIGN_LEFT) {		
		cpy = 5;
	}
	
	tabs = self->tabs;
	while (tabs) {
		Tab* tab;

		tab = (Tab*)tabs->entry;
		if (self->tabalignment == UI_ALIGN_TOP && x >= cpx &&
			x < cpx + tab->size.width +
				ui_value_px(&tab->margin.left, &tm) +
				ui_value_px(&tab->margin.right, &tm)) {
			rv = c;
			break;
		} else
		if ((self->tabalignment == UI_ALIGN_LEFT ||
				self->tabalignment == UI_ALIGN_RIGHT) &&
			y >= cpy && y < cpy + tab->size.height +
				ui_value_px(&tab->margin.top, &tm) +
				ui_value_px(&tab->margin.bottom, &tm)) {
			rv = c;
			break;
		}
		if (self->tabalignment == UI_ALIGN_TOP) {
			cpx += tab->size.width +
				ui_value_px(&tab->margin.left, &tm) +
				ui_value_px(&tab->margin.right, &tm);
		} else
		if (self->tabalignment == UI_ALIGN_LEFT ||
			self->tabalignment == UI_ALIGN_RIGHT) {
			cpy += tab->size.height +
				ui_value_px(&tab->margin.top, &tm) +
				ui_value_px(&tab->margin.bottom, &tm);
		}
		tabs = tabs->next;
		++c;
	}
	return rv;
}

void tabbar_onmousemove(TabBar* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	int tabindex;

	tabindex = tabbar_tabhittest(self, ev->x, ev->y);	
	if (tabindex != self->hoverindex) {
		self->hoverindex = tabindex;
		ui_component_invalidate(&self->component);
	}
	
}

void tabbar_onmouseenter(TabBar* self, psy_ui_Component* sender)
{
	self->hover = 1;
	ui_component_invalidate(&self->component);
}

void tabbar_onmouseleave(TabBar* self, psy_ui_Component* sender)
{		
	self->hover = 0;
	ui_component_invalidate(&self->component);
}

void tabbar_select(TabBar* self, int tab)
{
	self->selected = tab;
	ui_component_invalidate(&self->component);
	psy_signal_emit(&self->signal_change, self, 1, self->selected);
}

int tabbar_selected(TabBar* self)
{
	return self->selected;
}

Tab* tabbar_append(TabBar* self, const char* label)
{
	Tab* tab;

	tab = (Tab*)malloc(sizeof(Tab));
	tab_init(tab, label, 0);
	if (self->tabs != 0) {
		tab->margin.left = ui_value_makeew(1.5);
	} 
	psy_list_append(&self->tabs, tab);	
	tab->size = ui_component_textsize(&self->component, tab->text);
	tab->size.height = 20;

	return tab;
}

void tabbar_settabmargin(TabBar* self, int tabindex, const ui_margin* margin)
{
	psy_List* tabs;
	int c = 0;
	
	if (!margin) {
		return;
	}

	tabs = self->tabs;
	while (tabs) {
		Tab* tab;

		tab = (Tab*)tabs->entry;		
		if (c == tabindex) {
			tab->margin = *margin;
			break;
		}
		tabs = tabs->next;
		++c;
	}		
}

void tabbar_onalign(TabBar* self, psy_ui_Component* sender)
{
	psy_List* p;
	
	for (p = self->tabs; p != 0; p = p->next) {
		Tab* tab;

		tab = (Tab*)p->entry;
		tab->size = ui_component_textsize(&self->component, tab->text);
	}		
}

void tabbar_preferredsize(TabBar* self, ui_size* limit, ui_size* rv)
{
	if (rv) {
		ui_size size;
		ui_textmetric tm;
		psy_List* tabs;	
		int cpx = 0;
		int cpy = 0;
		int cpxmax = 0;
		int cpymax = 0;
		
		size = ui_component_size(&self->component);
		tm = ui_component_textmetric(&self->component);
		if (self->tabalignment == UI_ALIGN_TOP) {
			cpx = 0;
		} else 
		if (self->tabalignment == UI_ALIGN_RIGHT) {
			cpy = 5;
			cpx = 10;
		} else
		if (self->tabalignment == UI_ALIGN_LEFT) {		
			cpy = 5;
		}
		for (tabs = self->tabs; tabs != 0; tabs = tabs->next) {
			Tab* tab;
			ui_size tabsize;

			tab = (Tab*) tabs->entry;
			tabsize = ui_component_textsize(&self->component, tab->text);
			tabsize.height = (int) (tabsize.height * 1.5);
			tabsize.width = (int) tabsize.width;
			if (self->tabalignment == UI_ALIGN_TOP) {								
				cpx += tabsize.width;				
				cpx += ui_value_px(&tab->margin.left, &tm) +
						ui_value_px(&tab->margin.right, &tm);
				if (cpymax < cpy + tabsize.height +
						ui_value_px(&tab->margin.top, &tm) +
						ui_value_px(&tab->margin.bottom, &tm)) {
					cpymax = cpy + tabsize.height +
						ui_value_px(&tab->margin.top, &tm) +
						ui_value_px(&tab->margin.bottom, &tm);
				}
				if (cpxmax < cpx) {
					cpxmax = cpx;
				}
			} else 
			if (self->tabalignment == UI_ALIGN_LEFT ||
				self->tabalignment == UI_ALIGN_RIGHT) {
										
				cpy += tabsize.height;
				if (cpxmax < cpx + tabsize.width +
						ui_value_px(&tab->margin.left, &tm) +
						ui_value_px(&tab->margin.right, &tm)) {
					cpxmax = cpx + tabsize.width +
						ui_value_px(&tab->margin.left, &tm) +
						ui_value_px(&tab->margin.right, &tm);
				}
				if (cpymax < cpy + tabsize.height +
						ui_value_px(&tab->margin.top, &tm) +
						ui_value_px(&tab->margin.bottom, &tm)) {
					cpymax = cpy + tabsize.height +
						ui_value_px(&tab->margin.top, &tm) +
						ui_value_px(&tab->margin.bottom, &tm);
				}
			}
		}

		rv->width = cpxmax;
		rv->height = cpymax;
	}
}

Tab* tabbar_tab(TabBar* self, int tabindex)
{
	Tab* rv = 0;
	psy_List* p;
	int c = 0;
	
	for (p = self->tabs; p != 0 && c < tabindex; p = p->next, ++c);
	if (p) {		
		rv = (Tab*)p->entry;		
	}
	return rv;
}
