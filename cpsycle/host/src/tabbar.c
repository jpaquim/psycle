// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "tabbar.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../../detail/portable.h"

static void tabbar_ondraw(TabBar*, psy_ui_Graphics*);
static void tabbar_ondestroy(TabBar*, psy_ui_Component* component);
static void tabbar_onmousedown(TabBar*, psy_ui_MouseEvent*);
static void tabbar_onmousemove(TabBar*, psy_ui_MouseEvent*);
static void tabbar_onmouseenter(TabBar*);
static void tabbar_onmouseleave(TabBar*);
static int tabbar_tabhittest(TabBar* self, int x, int y, Tab** rvtab);
static void tabbar_onalign(TabBar*);
static void tabbar_onpreferredsize(TabBar*, psy_ui_Size* limit, psy_ui_Size* rv);
static void tabbar_resettabcheckstates(TabBar*);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(TabBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onalign = (psy_ui_fp_component_onalign) tabbar_onalign;
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			tabbar_onpreferredsize;
		vtable.ondraw = (psy_ui_fp_component_ondraw) tabbar_ondraw;
		vtable.onmousedown = (psy_ui_fp_component_onmousedown) tabbar_onmousedown;
		vtable.onmousemove = (psy_ui_fp_component_onmousemove) tabbar_onmousemove;
		vtable.onmouseenter = (psy_ui_fp_component_onmouseenter) tabbar_onmouseenter;
		vtable.onmouseleave = (psy_ui_fp_component_onmouseleave) tabbar_onmouseleave;
		vtable_initialized = 1;
	}
}

void tab_init(Tab* self, const char* text, psy_ui_Size* size,
	const psy_ui_Margin* margin)
{
	self->text = strdup(text);
	self->istoggle = FALSE;
	self->mode = TABMODE_SINGLESEL;
	self->checkstate = 0;
	if (size) {
		self->size = *size;
	}
	if (margin) {
		self->margin = *margin;
	} else {
		psy_ui_margin_init_all(&self->margin,
			psy_ui_value_makepx(0), psy_ui_value_makepx(0),
			psy_ui_value_makepx(0), psy_ui_value_makepx(0));
	}
}

void tab_dispose(Tab* self)
{
	free(self->text);
}

void tab_settext(Tab* self, const char* text)
{
	psy_strreset(&self->text, text);	
}

void tabbar_init(TabBar* self, psy_ui_Component* parent)
{
	psy_ui_component_init(tabbar_base(self), parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(tabbar_base(self));
	psy_ui_component_enablealign(tabbar_base(self));
	self->tabs = 0;
	self->selected = 0;
	self->hover = 0;
	self->hoverindex = -1;
	psy_signal_init(&self->signal_change);	
	psy_signal_connect(&tabbar_base(self)->signal_destroy, self,
		tabbar_ondestroy);
	self->tabalignment = psy_ui_ALIGN_TOP;
	psy_ui_margin_init_all(&self->defaulttabmargin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(1.5),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0));
}

void tabbar_ondestroy(TabBar* self, psy_ui_Component* component)
{	
	psy_List* p;	

	for (p = self->tabs; p != NULL; p = p->next) {	
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
	int tabheight;
		
	size = psy_ui_component_size(tabbar_base(self));
	tm = psy_ui_component_textmetric(tabbar_base(self));
	tabheight = psy_ui_value_px(&size.height, &tm);
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);	
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
		if (self->tabalignment == psy_ui_ALIGN_LEFT ||
			self->tabalignment == psy_ui_ALIGN_RIGHT) {
			cpx = psy_ui_value_px(&tab->margin.left, &tm);
		}
		if (self->selected == c) {
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00B1C8B0));
			cpxsel = cpx + psy_ui_value_px(&tab->margin.left, &tm);
			selwidth = psy_ui_value_px(&tab->size.width, &tm);
		}
		if (self->hover && self->hoverindex == c && self->hoverindex != self->selected) {
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00EAEAEA));
			cpxhover = cpx + psy_ui_value_px(&tab->margin.left, &tm);
			hoverwidth = psy_ui_value_px(&tab->size.width, &tm);
		} else
		if (tab->istoggle) {
			if (tab->checkstate) {
				psy_ui_settextcolour(g, psy_ui_colour_make(0x00B1C8B0));
			} else {
				psy_ui_settextcolour(g, psy_ui_colour_make(0x00D1C5B6));
			}
		} else		
		if (self->selected == c) {			
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00B1C8B0));
		} else {
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00D1C5B6));
		}		
		if (self->tabalignment == psy_ui_ALIGN_TOP) {			
			cpx += psy_ui_value_px(&tab->margin.left, &tm);
			if (tab->mode == TABMODE_LABEL) {
				psy_ui_settextcolour(g, psy_ui_colour_make(0x00B1A596));
			}
			if (self->selected == c && self->tabalignment == psy_ui_ALIGN_TOP) {
				int y;
				// psy_ui_Rectangle r;

				psy_ui_setcolour(g, psy_ui_colour_make(0x00B1C8B0));
				y = tm.tmHeight + 2;

				// psy_ui_setrectangle(&r, cpxsel, 0, selwidth, y + 1);
				// psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x002F3E25));				

				// psy_ui_drawline(g, cpxsel, y, cpxsel + selwidth, y);
				if (self->hover && self->hoverindex != self->selected) {
					psy_ui_setcolour(g, psy_ui_colour_make(0x00FFFFFF));
					psy_ui_drawline(g, cpxhover, y, cpxhover + hoverwidth, y);
				}
			}
			psy_ui_textout(g, cpx, cpy, tab->text, strlen(tab->text));
			cpx += psy_ui_value_px(&tab->size.width, &tm) + psy_ui_value_px(&tab->margin.right, &tm);
		} else
		if (self->tabalignment == psy_ui_ALIGN_LEFT ||
				self->tabalignment == psy_ui_ALIGN_RIGHT) {
			cpx = psy_ui_value_px(&tab->margin.left, &tm);
			if (self->hover && self->hoverindex == c) {
				
				// psy_ui_setcolour(g, 0x00383838);
				// psy_ui_drawrectangle(g, r);
				psy_ui_setcolour(g, psy_ui_colour_make(0x00FFFFFF));
				psy_ui_drawline(g,
					cpx,
					cpy + psy_ui_value_px(&tab->size.height, &tm) + psy_ui_value_px(&tab->margin.top, &tm),
					cpx + hoverwidth,
					cpy + psy_ui_value_px(&tab->size.height, &tm) + psy_ui_value_px(&tab->margin.top, &tm));
			}			
			cpy += psy_ui_value_px(&tab->margin.top, &tm);
			if (tab->mode == TABMODE_LABEL) {
				psy_ui_settextcolour(g, psy_ui_colour_make(0x00666666));
			}			
			psy_ui_textout(g, cpx, cpy, tab->text, strlen(tab->text));			
			cpy += psy_ui_value_px(&tab->size.height, &tm) + psy_ui_value_px(&tab->margin.bottom, &tm);
		}
	}
	psy_ui_setcolour(g, psy_ui_colour_make(0x005F5F5F));
	if (self->tabalignment == psy_ui_ALIGN_TOP) {
		int y;		

		psy_ui_setcolour(g, psy_ui_colour_make(0x00B1C8B0));
		y = tm.tmHeight + 2;				
		psy_ui_drawline(g, cpxsel, y, cpxsel + selwidth, y);		 
		if (self->hover && self->hoverindex != self->selected) {		
			psy_ui_setcolour(g, psy_ui_colour_make(0x00FFFFFF));
			psy_ui_drawline(g, cpxhover, y, cpxhover + hoverwidth, y);
		}
	} else
	if (self->tabalignment == psy_ui_ALIGN_LEFT) {
		psy_ui_drawline(g, psy_ui_value_px(&size.width, &tm) - 2, cpx,
			psy_ui_value_px(&size.width, &tm) - 2, cpy);
	}
	
}

void tabbar_onmousedown(TabBar* self, psy_ui_MouseEvent* ev)
{	
	int tabindex;
	Tab* tab = 0;

	tabindex = tabbar_tabhittest(self, ev->x, ev->y, &tab);
	if (tab && tab->mode == TABMODE_LABEL) {
		return;
	}
	if (tab && tabindex != -1 && (tabindex != self->selected || tab->istoggle)) {
		if (!tab->istoggle) {
			self->selected = tabindex;
		} else {
			tab->checkstate = !tab->checkstate;
		}
		psy_ui_component_invalidate(tabbar_base(self));
		psy_signal_emit(&self->signal_change, self, 1, tabindex);
	}

}

int tabbar_tabhittest(TabBar* self, int x, int y, Tab** rvtab)
{
	psy_List* tabs;
	int rv = -1;
	int c = 0;
	int cpx = 0;
	int cpy = 0;
	psy_ui_TextMetric tm;

	*rvtab = NULL;
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
			x < cpx + psy_ui_value_px(&tab->size.width, &tm) +
				psy_ui_value_px(&tab->margin.left, &tm) +
				psy_ui_value_px(&tab->margin.right, &tm)) {
			*rvtab = tab;
			rv = c;
			break;
		} else
		if ((self->tabalignment == psy_ui_ALIGN_LEFT ||
				self->tabalignment == psy_ui_ALIGN_RIGHT) &&
			y >= cpy && y < cpy + psy_ui_value_px(&tab->size.height, &tm) +
				psy_ui_value_px(&tab->margin.top, &tm) +
				psy_ui_value_px(&tab->margin.bottom, &tm)) {
			*rvtab = tab;
			rv = c;
			break;
		}
		if (self->tabalignment == psy_ui_ALIGN_TOP) {
			cpx += psy_ui_value_px(&tab->size.width, &tm) +
				psy_ui_value_px(&tab->margin.left, &tm) +
				psy_ui_value_px(&tab->margin.right, &tm);
		} else
		if (self->tabalignment == psy_ui_ALIGN_LEFT ||
			self->tabalignment == psy_ui_ALIGN_RIGHT) {
			cpy += psy_ui_value_px(&tab->size.height, &tm) +
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
	Tab* tab = 0;

	tabindex = tabbar_tabhittest(self, ev->x, ev->y, &tab);	
	if (tab && tab->mode == TABMODE_LABEL) {
		self->hoverindex = -1;
		psy_ui_component_invalidate(tabbar_base(self));
		return;
	} else
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

void tabbar_select(TabBar* self, int tabindex)
{
	Tab* tab;

	self->selected = tabindex;
	tab = tabbar_tab(self, tabindex);
	if (tab) {
		tab->checkstate = 1;
	}
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

	tab = (Tab*) malloc(sizeof(Tab));
	if (tab) {
		tab_init(tab, label, 0, &self->defaulttabmargin);
		if (self->tabs == 0) {
			if (self->tabalignment == psy_ui_ALIGN_TOP) {
				tab->margin.left = psy_ui_value_makepx(0);				
			} else {
				// tab->margin.top = psy_ui_value_makepx(0);
			}			
		}
		psy_list_append(&self->tabs, tab);
		tab->size = psy_ui_component_textsize(tabbar_base(self), tab->text);
		tab->size.height = psy_ui_value_makeeh(1.5);
	}
	return tab;
}

void tabbar_append_tabs(TabBar* self, const char* label, ...)
{
	const char* currlabel;
	va_list ap;
	
	va_start(ap, label);
	for (currlabel = label; currlabel != NULL; currlabel = va_arg(ap, const char*)) {
		tabbar_append(self, currlabel);		
	}
	va_end(ap);
}

void tabbar_clear(TabBar* self)
{
	psy_List* p;

	for (p = self->tabs; p != NULL; p = p->next) {
		tab_dispose((Tab*)(p->entry));
		free(p->entry);
	}
	psy_list_free(self->tabs);
	self->tabs = 0;
	self->selected = 0;
}

void tabbar_rename_tabs(TabBar* self, const char* label, ...)
{
	const char* currlabel;
	psy_List* t;
	va_list ap;

	va_start(ap, label);
	for (t = self->tabs, currlabel = label; t != NULL && currlabel != NULL;
			t = t->next, currlabel = va_arg(ap, const char*)) {
		Tab* tab;

		tab = (Tab*)t->entry;
		tab_settext(tab, currlabel);
	}
	va_end(ap);
}

void tabbar_settabmargin(TabBar* self, int tabindex,
	const psy_ui_Margin* margin)
{
	Tab* tab;

	tab = tabbar_tab(self, tabindex);
	if (tab) {
		if (margin) {
			tab->margin = *margin;
		} else {
			tab->margin = self->defaulttabmargin;
		}
	}		
}

void tabbar_settabmode(TabBar* self, int tabindex, TabMode mode)
{
	Tab* tab;

	tab = tabbar_tab(self, tabindex);
	if (tab) {
		tab->mode = mode;
		tab->istoggle = (mode == TABMODE_MULTISEL);
	}
}

void tabbar_onalign(TabBar* self)
{
	psy_List* p;
	
	for (p = self->tabs; p != NULL; p = p->next) {
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
			tabsize.height = psy_ui_value_makeeh(1.5);
			if (self->tabalignment == psy_ui_ALIGN_TOP) {								
				cpx += psy_ui_value_px(&tabsize.width, &tm);
				cpx += psy_ui_value_px(&tab->margin.left, &tm) +
						psy_ui_value_px(&tab->margin.right, &tm);
				if (cpymax < cpy + psy_ui_value_px(&tabsize.height, &tm) +
						psy_ui_value_px(&tab->margin.top, &tm) +
						psy_ui_value_px(&tab->margin.bottom, &tm)) {
					cpymax = cpy + psy_ui_value_px(&tabsize.height, &tm) +
						psy_ui_value_px(&tab->margin.top, &tm) +
						psy_ui_value_px(&tab->margin.bottom, &tm);
				}
				if (cpxmax < cpx) {
					cpxmax = cpx;
				}
			} else 
			if (self->tabalignment == psy_ui_ALIGN_LEFT ||
				self->tabalignment == psy_ui_ALIGN_RIGHT) {
										
				cpy += psy_ui_value_px(&tabsize.height, &tm);
				if (cpxmax < cpx + psy_ui_value_px(&tabsize.width, &tm) +
						psy_ui_value_px(&tab->margin.left, &tm) +
						psy_ui_value_px(&tab->margin.right, &tm)) {
					cpxmax = cpx + psy_ui_value_px(&tabsize.width, &tm) +
						psy_ui_value_px(&tab->margin.left, &tm) +
						psy_ui_value_px(&tab->margin.right, &tm);
				}
				if (cpymax < cpy + psy_ui_value_px(&tabsize.height, &tm) +
						psy_ui_value_px(&tab->margin.top, &tm) +
						psy_ui_value_px(&tab->margin.bottom, &tm)) {
					cpymax = cpy + psy_ui_value_px(&tabsize.height, &tm) +
						psy_ui_value_px(&tab->margin.top, &tm) +
						psy_ui_value_px(&tab->margin.bottom, &tm);
				}
			}
		}

		rv->width = psy_ui_value_makepx(cpxmax);
		rv->height = psy_ui_value_makepx(cpymax);
	}
}

Tab* tabbar_tab(TabBar* self, int tabindex)
{
	Tab* rv = 0;
	psy_List* p;
	int c = 0;
	
	for (p = self->tabs; p != NULL && c < tabindex; p = p->next, ++c);
	if (p) {		
		rv = (Tab*)p->entry;		
	}
	return rv;
}

void tabbar_setdefaulttabmargin(TabBar* self, const psy_ui_Margin* margin)
{
	self->defaulttabmargin = *margin;
}

int tabbar_checkstate(TabBar* self, int tabindex)
{	
	Tab* tab;

	tab = tabbar_tab(self, tabindex);
	if (tab) {
		return tab->checkstate;		
	}
	return 0;
}

int tabbar_numchecked(TabBar* self)
{
	int rv;
	psy_List* tabs;
	
	for (rv = 0, tabs = self->tabs; tabs != NULL; tabs = tabs->next) {
		Tab* tab;

		tab = (Tab*)tabs->entry;
		if (tab != NULL && tab->checkstate != 0) {
			++rv;
		}		
	}
	return rv;
}

void tabbar_resettabcheckstates(TabBar* self)
{
	int rv;
	psy_List* tabs;

	for (rv = 0, tabs = self->tabs; tabs != NULL; tabs = tabs->next) {
		Tab* tab;

		tab = (Tab*)tabs->entry;
		if (tab != NULL) {
			tab->checkstate = 0;
		}
	}
}