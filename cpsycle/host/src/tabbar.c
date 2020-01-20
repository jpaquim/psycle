// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "tabbar.h"

#include <stdlib.h>
#include <string.h>

static void tabbar_ondraw(TabBar*, psy_ui_Graphics*);
static void tabbar_ondestroy(TabBar*, psy_ui_Component* component);
static void tabbar_onmousedown(TabBar*, psy_ui_MouseEvent*);
static void tabbar_onmousemove(TabBar*, psy_ui_MouseEvent*);
static void tabbar_onmouseenter(TabBar*);
static void tabbar_onmouseleave(TabBar*);
static int tabbar_tabhittest(TabBar* self, int x, int y);
static void tabbar_onalign(TabBar*, psy_ui_Component* sender);
static void tabbar_onpreferredsize(TabBar*, psy_ui_Size* limit, psy_ui_Size* rv);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(TabBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			tabbar_onpreferredsize;
		vtable.ondraw = (psy_ui_fp_ondraw) tabbar_ondraw;
		vtable.onmousedown = (psy_ui_fp_onmousedown) tabbar_onmousedown;
		vtable.onmousemove = (psy_ui_fp_onmousemove) tabbar_onmousemove;
		vtable.onmouseenter = (psy_ui_fp_onmouseenter) tabbar_onmouseenter;
		vtable.onmouseleave = (psy_ui_fp_onmouseleave) tabbar_onmouseleave;
		vtable_initialized = 1;
	}
}

void tab_init(Tab* self, const char* text, psy_ui_Size* size)
{
	self->text = _strdup(text);
	if (size) {
		self->size = *size;
	}	
	psy_ui_margin_init(&self->margin,
		psy_ui_value_makepx(0),
		psy_ui_value_makepx(0),
		psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
}

void tab_dispose(Tab* self)
{
	free(self->text);
}

void tabbar_init(TabBar* self, psy_ui_Component* parent)
{
	psy_ui_component_init(tabbar_base(self), parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(tabbar_base(self));
	self->tabs = 0;
	self->selected = 0;
	self->hover = 0;
	self->hoverindex = -1;
	self->tabalignment = psy_ui_ALIGN_TOP;	
	psy_signal_init(&self->signal_change);	
	psy_signal_connect(&tabbar_base(self)->signal_destroy, self,
		tabbar_ondestroy);
	psy_signal_connect(&tabbar_base(self)->signal_align, self, tabbar_onalign);	
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

void tabbar_ondraw(TabBar* self, psy_ui_Graphics* g)
{	
	psy_List* tabs;
	int c = 0;
	int cpx = 0;
	int cpy = 0;
	int cpxsel = 0;
	int selwidth = 0;	
	int cpxhover = 0;
	int hoverwidth = 0;
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	
	size = psy_ui_component_size(tabbar_base(self));
	tm = psy_ui_component_textmetric(tabbar_base(self));
	psy_ui_setbackgroundmode(g, TRANSPARENT);	
	if (self->tabalignment == psy_ui_ALIGN_TOP) {
		cpx = 0;
	} else 
	if (self->tabalignment == psy_ui_ALIGN_RIGHT) {
		cpy = 5;
		cpx = 10;
	} else
	if (self->tabalignment == psy_ui_ALIGN_LEFT) {		
		cpy = 5;
	}
	for (tabs = self->tabs; tabs != 0; tabs = tabs->next, ++c) {
		Tab* tab;

		tab = (Tab*)tabs->entry;
		if (self->selected == c) {
			psy_ui_settextcolor(g, 0x00B1C8B0);
			cpxsel = cpx + psy_ui_value_px(&tab->margin.left, &tm);
			selwidth = tab->size.width;
		}
		if (self->hover && self->hoverindex == c && self->hoverindex != self->selected) {
			psy_ui_settextcolor(g, 0x00FFFFFF);
			cpxhover = cpx + psy_ui_value_px(&tab->margin.left, &tm);
			hoverwidth = tab->size.width;
		} else
		if (self->selected == c) {
			psy_ui_settextcolor(g, 0x00B1C8B0);		
		} else {
			psy_ui_settextcolor(g, 0x00D1C5B6);
		}		
		if (self->tabalignment == psy_ui_ALIGN_TOP) {
			cpx += psy_ui_value_px(&tab->margin.left, &tm);
			psy_ui_textout(g, cpx, cpy, tab->text, strlen(tab->text));
			cpx += tab->size.width + psy_ui_value_px(&tab->margin.right, &tm);
		} else
		if (self->tabalignment == psy_ui_ALIGN_LEFT ||
			self->tabalignment == psy_ui_ALIGN_RIGHT) {
			cpy += psy_ui_value_px(&tab->margin.top, &tm);
			psy_ui_textout(g, cpx, cpy, tab->text, strlen(tab->text));
			cpy += tab->size.height + psy_ui_value_px(&tab->margin.bottom, &tm);
		}
	}
	psy_ui_setcolor(g, 0x005F5F5F);		
	if (self->tabalignment == psy_ui_ALIGN_TOP) {
		int y;

		psy_ui_setcolor(g, 0x00B1C8B0);
		y = tm.tmHeight + 2;
		psy_ui_drawline(g, cpxsel, y, cpxsel + selwidth, y);		 
		if (self->hover && self->hoverindex != self->selected) {		
			psy_ui_setcolor(g, 0x00FFFFFF);
			psy_ui_drawline(g, cpxhover, y, cpxhover + hoverwidth, y);
		}
	} else
	if (self->tabalignment == psy_ui_ALIGN_LEFT) {
		psy_ui_drawline(g, size.width - 2, 0, size.width - 2, cpy);
	}
	if (self->tabalignment == psy_ui_ALIGN_RIGHT) {
		psy_ui_drawline(g, 0, 0, 0, cpy);
	}
}

void tabbar_onmousedown(TabBar* self, psy_ui_MouseEvent* ev)
{
	int tabindex;
	
	tabindex = tabbar_tabhittest(self, ev->x, ev->y);
	if (tabindex != -1 && tabindex != self->selected)  {
		self->selected = tabbar_tabhittest(self, ev->x, ev->y);
		psy_ui_component_invalidate(tabbar_base(self));
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
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(tabbar_base(self));
	if (self->tabalignment == psy_ui_ALIGN_TOP) {
		cpx = 0;
	} else 
	if (self->tabalignment == psy_ui_ALIGN_RIGHT) {
		cpy = 5;
		cpx = 10;
	} else
	if (self->tabalignment == psy_ui_ALIGN_LEFT) {		
		cpy = 5;
	}
	
	tabs = self->tabs;
	while (tabs) {
		Tab* tab;

		tab = (Tab*)tabs->entry;
		if (self->tabalignment == psy_ui_ALIGN_TOP && x >= cpx &&
			x < cpx + tab->size.width +
				psy_ui_value_px(&tab->margin.left, &tm) +
				psy_ui_value_px(&tab->margin.right, &tm)) {
			rv = c;
			break;
		} else
		if ((self->tabalignment == psy_ui_ALIGN_LEFT ||
				self->tabalignment == psy_ui_ALIGN_RIGHT) &&
			y >= cpy && y < cpy + tab->size.height +
				psy_ui_value_px(&tab->margin.top, &tm) +
				psy_ui_value_px(&tab->margin.bottom, &tm)) {
			rv = c;
			break;
		}
		if (self->tabalignment == psy_ui_ALIGN_TOP) {
			cpx += tab->size.width +
				psy_ui_value_px(&tab->margin.left, &tm) +
				psy_ui_value_px(&tab->margin.right, &tm);
		} else
		if (self->tabalignment == psy_ui_ALIGN_LEFT ||
			self->tabalignment == psy_ui_ALIGN_RIGHT) {
			cpy += tab->size.height +
				psy_ui_value_px(&tab->margin.top, &tm) +
				psy_ui_value_px(&tab->margin.bottom, &tm);
		}
		tabs = tabs->next;
		++c;
	}
	return rv;
}

void tabbar_onmousemove(TabBar* self, psy_ui_MouseEvent* ev)
{	
	int tabindex;

	tabindex = tabbar_tabhittest(self, ev->x, ev->y);	
	if (tabindex != self->hoverindex) {
		self->hoverindex = tabindex;
		psy_ui_component_invalidate(tabbar_base(self));
	}
	
}

void tabbar_onmouseenter(TabBar* self)
{
	self->hover = 1;
	psy_ui_component_invalidate(tabbar_base(self));
}

void tabbar_onmouseleave(TabBar* self)
{		
	self->hover = 0;
	psy_ui_component_invalidate(tabbar_base(self));
}

void tabbar_select(TabBar* self, int tab)
{
	self->selected = tab;
	psy_ui_component_invalidate(tabbar_base(self));
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
		tab->margin.left = psy_ui_value_makeew(1.5);
	} 
	psy_list_append(&self->tabs, tab);	
	tab->size = psy_ui_component_textsize(tabbar_base(self), tab->text);
	tab->size.height = 20;

	return tab;
}

void tabbar_settabmargin(TabBar* self, int tabindex,
	const psy_ui_Margin* margin)
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
		tab->size = psy_ui_component_textsize(tabbar_base(self), tab->text);
	}		
}

void tabbar_onpreferredsize(TabBar* self, psy_ui_Size* limit, psy_ui_Size* rv)
{
	if (rv) {
		psy_ui_Size size;
		psy_ui_TextMetric tm;
		psy_List* tabs;	
		int cpx = 0;
		int cpy = 0;
		int cpxmax = 0;
		int cpymax = 0;
		
		size = psy_ui_component_size(tabbar_base(self));
		tm = psy_ui_component_textmetric(tabbar_base(self));
		if (self->tabalignment == psy_ui_ALIGN_TOP) {
			cpx = 0;
		} else 
		if (self->tabalignment == psy_ui_ALIGN_RIGHT) {
			cpy = 5;
			cpx = 10;
		} else
		if (self->tabalignment == psy_ui_ALIGN_LEFT) {		
			cpy = 5;
		}
		for (tabs = self->tabs; tabs != 0; tabs = tabs->next) {
			Tab* tab;
			psy_ui_Size tabsize;

			tab = (Tab*) tabs->entry;
			tabsize = psy_ui_component_textsize(tabbar_base(self), tab->text);
			tabsize.height = (int) (tabsize.height * 1.5);
			tabsize.width = (int) tabsize.width;
			if (self->tabalignment == psy_ui_ALIGN_TOP) {								
				cpx += tabsize.width;				
				cpx += psy_ui_value_px(&tab->margin.left, &tm) +
						psy_ui_value_px(&tab->margin.right, &tm);
				if (cpymax < cpy + tabsize.height +
						psy_ui_value_px(&tab->margin.top, &tm) +
						psy_ui_value_px(&tab->margin.bottom, &tm)) {
					cpymax = cpy + tabsize.height +
						psy_ui_value_px(&tab->margin.top, &tm) +
						psy_ui_value_px(&tab->margin.bottom, &tm);
				}
				if (cpxmax < cpx) {
					cpxmax = cpx;
				}
			} else 
			if (self->tabalignment == psy_ui_ALIGN_LEFT ||
				self->tabalignment == psy_ui_ALIGN_RIGHT) {
										
				cpy += tabsize.height;
				if (cpxmax < cpx + tabsize.width +
						psy_ui_value_px(&tab->margin.left, &tm) +
						psy_ui_value_px(&tab->margin.right, &tm)) {
					cpxmax = cpx + tabsize.width +
						psy_ui_value_px(&tab->margin.left, &tm) +
						psy_ui_value_px(&tab->margin.right, &tm);
				}
				if (cpymax < cpy + tabsize.height +
						psy_ui_value_px(&tab->margin.top, &tm) +
						psy_ui_value_px(&tab->margin.bottom, &tm)) {
					cpymax = cpy + tabsize.height +
						psy_ui_value_px(&tab->margin.top, &tm) +
						psy_ui_value_px(&tab->margin.bottom, &tm);
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

psy_ui_Component* tabbar_base(TabBar* self)
{
	return &self->component;
}
