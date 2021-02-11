// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "tabbar.h"

// platform
#include "../../detail/portable.h"

// Tab
// implementation
void tab_init(Tab* self, const char* text, psy_ui_Size* size,
	const psy_ui_Margin* margin)
{
	assert(self);

	self->text = strdup(text);
	self->translation = NULL;
	psy_strreset(&self->translation, psy_ui_translate(text));
	self->istoggle = FALSE;
	self->mode = TABMODE_SINGLESEL;
	self->checkstate = 0;
	self->position = psy_ui_realpoint_make(0.0, 0.0);
	if (size) {
		self->size = *size;
	}
	if (margin) {
		self->margin = *margin;
	} else {
		psy_ui_margin_init(&self->margin);			
	}
}

void tab_dispose(Tab* self)
{
	assert(self);

	free(self->text);
	self->text = NULL;
	free(self->translation);
	self->translation = NULL;
}

void tab_settext(Tab* self, const char* text)
{
	assert(self);

	psy_strreset(&self->text, text);	
	psy_strreset(&self->translation, psy_ui_translate(text));
}

psy_ui_RealRectangle tab_position(const Tab* self, const psy_ui_TextMetric* tm)
{
	assert(self);
		
	return psy_ui_realrectangle_make(self->position,				
		psy_ui_realsize_make(psy_ui_value_px(&self->size.width, tm),
			psy_ui_value_px(&self->size.height, tm)));
}


// TabBarSkin
void tabbarskin_init(TabBarSkin* self)
{
	self->text = psy_ui_style(psy_ui_STYLE_TAB)->colour;	
}

// TabBar
// prototypes
static void tabbar_ondestroy(TabBar*);
static void tabbar_onlanguagechanged(TabBar*);
static void tabbar_ondraw(TabBar*, psy_ui_Graphics*);
static void tabbar_drawtab(TabBar*, psy_ui_Graphics*,
	Tab* tab, bool hover, bool selected, bool drawselline);
static psy_ui_RealSize tabbar_calctabpositions(TabBar*);
static void tabbar_onmousedown(TabBar*, psy_ui_MouseEvent*);
static void tabbar_onmousemove(TabBar*, psy_ui_MouseEvent*);
static void tabbar_onmouseenter(TabBar*);
static void tabbar_onmouseleave(TabBar*);
static uintptr_t tabbar_tabhittest(TabBar* self, double x, double y,
	Tab** rvtab);
static void tabbar_onalign(TabBar*);
static void tabbar_onpreferredsize(TabBar*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void tabbar_resettabcheckstates(TabBar*);
static void tabbar_onupdatestyles(TabBar*);
static void tabbar_drawtabbackground(TabBar*, psy_ui_Graphics*, Tab*,
	psy_ui_Style*, psy_ui_RealPoint);

static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;
// vtable
static void vtable_init(TabBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);		
		vtable.ondestroy = (psy_ui_fp_component_ondestroy)tabbar_ondestroy;
		vtable.onalign = (psy_ui_fp_component_onalign)tabbar_onalign;
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			tabbar_onpreferredsize;
		vtable.ondraw = (psy_ui_fp_component_ondraw)tabbar_ondraw;
		vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			tabbar_onmousedown;
		vtable.onmousemove = (psy_ui_fp_component_onmouseevent)
			tabbar_onmousemove;
		vtable.onmouseenter = (psy_ui_fp_component_onmouseenter)
			tabbar_onmouseenter;
		vtable.onmouseleave = (psy_ui_fp_component_onmouseleave)
			tabbar_onmouseleave;
		vtable.onlanguagechanged = (psy_ui_fp_component_onlanguagechanged)
			tabbar_onlanguagechanged;
		vtable.onupdatestyles = (psy_ui_fp_component_onupdatestyles)
			tabbar_onupdatestyles;
		vtable_initialized = TRUE;
	}
}
// implementation
void tabbar_init(TabBar* self, psy_ui_Component* parent)
{
	assert(self);

	psy_ui_component_init(tabbar_base(self), parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(tabbar_base(self));	
	psy_signal_init(&self->signal_change);
	self->tabs = NULL;
	self->selected = 0;
	self->hover = FALSE;
	self->hoverindex = psy_INDEX_INVALID;
	self->tabalignment = psy_ui_ALIGN_TOP;
	psy_ui_style_init_default(&self->style_tab, psy_ui_STYLE_TAB);
	psy_ui_style_init_default(&self->style_tab_hover, psy_ui_STYLE_TAB_HOVER);
	psy_ui_style_init_default(&self->style_tab_select, psy_ui_STYLE_TAB_SELECT);
	psy_ui_style_init_default(&self->style_tab_label, psy_ui_STYLE_COMMON);
	self->defaulttabheight = psy_ui_value_makeeh(1.8);
	tabbarskin_init(&self->skin);		
	psy_ui_margin_init_all_em(&self->defaulttabmargin, 0.0, 2.0, 0.0, 0.0);		
	tabbar_onupdatestyles(self);
}

void tabbar_ondestroy(TabBar* self)
{	
	assert(self);

	psy_list_deallocate(&self->tabs, (psy_fp_disposefunc)tab_dispose);	
	psy_signal_dispose(&self->signal_change);
	psy_ui_style_dispose(&self->style_tab);
	psy_ui_style_dispose(&self->style_tab_hover);
	psy_ui_style_dispose(&self->style_tab_select);
}

void tabbar_ondraw(TabBar* self, psy_ui_Graphics* g)
{	
	psy_List* tabs;
	intptr_t c = 0;	
			
	assert(self);

	tabbar_calctabpositions(self);
	for (tabs = self->tabs; tabs != 0; psy_list_next(&tabs), ++c) {
		Tab* tab;		

		tab = (Tab*)psy_list_entry(tabs);		
		tabbar_drawtab(self, g, tab,
			self->hover && self->hoverindex == c &&
			self->hoverindex != self->selected,
			self->selected == c,
			self->tabalignment == psy_ui_ALIGN_TOP ||
			self->tabalignment == psy_ui_ALIGN_BOTTOM);
	}
}

psy_ui_RealSize tabbar_calctabpositions(TabBar* self)
{
	psy_List* tabs;
	intptr_t c = 0;
	psy_ui_RealPoint currpos;
	psy_ui_RealSize maxsize;
	double cpy = 0;
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;

	assert(self);

	size = psy_ui_component_sizepx(tabbar_base(self));
	tm = psy_ui_component_textmetric(tabbar_base(self));	
	currpos = psy_ui_realpoint_make(0.0, 0.0);
	maxsize = psy_ui_realsize_make(0.0, 0.0);
	if (self->tabalignment == psy_ui_ALIGN_TOP) {
		currpos.x = 0.0;
	} else if (self->tabalignment == psy_ui_ALIGN_RIGHT) {
		currpos.y = 5.0;
		currpos.x = 10.0;
	} else if (self->tabalignment == psy_ui_ALIGN_LEFT) {
		currpos.y = 5.0;
	}
	for (tabs = self->tabs; tabs != 0; psy_list_next(&tabs), ++c) {
		Tab* tab;
		psy_ui_RealPoint position;		
		
		tab = (Tab*)psy_list_entry(tabs);		
		if (self->tabalignment == psy_ui_ALIGN_TOP) {
			currpos.x += psy_ui_value_px(&tab->margin.left, tm);
			position = currpos;			
			currpos.x += psy_ui_value_px(&tab->size.width, tm) +
				psy_ui_value_px(&tab->margin.right, tm);

			if (maxsize.height < currpos.y + psy_ui_value_px(&tab->size.height, tm) +
				psy_ui_value_px(&tab->margin.top, tm) +
				psy_ui_value_px(&tab->margin.bottom, tm)) {
				maxsize.height = currpos.y + psy_ui_value_px(&tab->size.height, tm) +
					psy_ui_value_px(&tab->margin.top, tm) +
					psy_ui_value_px(&tab->margin.bottom, tm);
			}
			if (maxsize.width < currpos.x) {
				maxsize.width = currpos.x;
			}
		} else if (self->tabalignment == psy_ui_ALIGN_LEFT ||
			self->tabalignment == psy_ui_ALIGN_RIGHT) {
			currpos.x = psy_ui_value_px(&tab->margin.left, tm);
			currpos.y += psy_ui_value_px(&tab->margin.top, tm);
			position = currpos;
			currpos.y += psy_ui_value_px(&tab->size.height, tm) +
				psy_ui_value_px(&tab->margin.bottom, tm);

			if (maxsize.width < currpos.x + psy_ui_value_px(&tab->size.width, tm) +
				psy_ui_value_px(&tab->margin.left, tm) +
				psy_ui_value_px(&tab->margin.right, tm)) {
				maxsize.width = currpos.x + psy_ui_value_px(&tab->size.width, tm) +
					psy_ui_value_px(&tab->margin.left, tm) +
					psy_ui_value_px(&tab->margin.right, tm);
			}
			if (maxsize.height < currpos.y + psy_ui_value_px(&tab->size.height, tm) +
				psy_ui_value_px(&tab->margin.top, tm) +
				psy_ui_value_px(&tab->margin.bottom, tm)) {
				maxsize.height = currpos.y + psy_ui_value_px(&tab->size.height, tm) +
					psy_ui_value_px(&tab->margin.top, tm) +
					psy_ui_value_px(&tab->margin.bottom, tm);
			}

		} else {
			position = currpos;
		}
		tab->position = position;		
	}
	return maxsize;
}

void tabbar_drawtab(TabBar* self, psy_ui_Graphics* g,
	Tab* tab, bool hover, bool selected, bool drawselline)
{
	char* text;
	const psy_ui_TextMetric* tm;
	psy_ui_RealPoint position;
	double centery;

	assert(self);
	assert(tab);

	position = tab->position;
	tm = psy_ui_component_textmetric(tabbar_base(self));
	centery = (psy_ui_value_px(&tab->size.height, tm) - tm->tmHeight) / 2;
	if (tab->translation) {
		text = tab->translation;
	} else {
		text = tab->text;
	}
	if (tab->mode == TABMODE_LABEL) {
		psy_ui_settextcolour(g, self->style_tab_label.colour);
	} else if (tab->istoggle) {
		if (tab->checkstate) {
			psy_ui_settextcolour(g, self->style_tab_select.colour);
		} else {
			psy_ui_settextcolour(g, self->style_tab.colour);
		}
	} else if (selected) {
		psy_ui_settextcolour(g, self->style_tab_select.colour);
	} else if (hover) {
		psy_ui_settextcolour(g, self->style_tab_hover.colour);
	} else {
		psy_ui_settextcolour(g, self->style_tab.colour);
	}	
	if (hover) {
		double width;

		tabbar_drawtabbackground(self, g, tab, &self->style_tab_hover,
			position);
		width = psy_ui_value_px(&tab->size.width, tm);		
		psy_ui_drawborder(g,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(position.x, position.y),
				psy_ui_realsize_make(
					psy_ui_value_px(&tab->size.width, tm),
					psy_ui_value_px(&tab->size.height, tm))),
				//psy_ui_realsize_make(
				//	width,
				//	centery + tm->tmHeight + 2)),
				self->style_tab_hover.border);				
	}
	psy_ui_textout(g, position.x, position.y + centery, text,
		psy_strlen(text));
	if (selected && drawselline) {
		double width;

		width = psy_ui_value_px(&tab->size.width, tm);
		psy_ui_setcolour(g, self->style_tab_select.backgroundcolour);
		psy_ui_drawline(g,
			psy_ui_realpoint_make(position.x, position.y +
				centery + tm->tmHeight + 2),
			psy_ui_realpoint_make(position.x + width, position.y +
				centery + tm->tmHeight + 2));
	}
}

void tabbar_drawtabbackground(TabBar* self, psy_ui_Graphics* g,
	Tab* tab, psy_ui_Style* style, psy_ui_RealPoint position)
{		
	if (style->backgroundcolour.mode.set) {
		psy_ui_RealRectangle r;
		const psy_ui_TextMetric* tm;		

		tm = psy_ui_component_textmetric(tabbar_base(self));
		r = psy_ui_realrectangle_make(
			position,
			psy_ui_realsize_make(
				psy_ui_value_px(&tab->size.width, tm),
				psy_ui_value_px(&tab->size.height, tm)));
		if (psy_ui_border_isround(&style->border)) {	
			psy_ui_drawsolidroundrectangle(g, r,
				psy_ui_size_make(
					style->border.border_bottom_left_radius,
					style->border.border_bottom_left_radius),
				style->backgroundcolour);
		} else {
			psy_ui_drawsolidrectangle(g, r, style->backgroundcolour);
		}
	}
}

void tabbar_onmousedown(TabBar* self, psy_ui_MouseEvent* ev)
{	
	intptr_t tabindex;
	Tab* tab;

	assert(self);

	tab = NULL;
	tabindex = tabbar_tabhittest(self, ev->pt.x, ev->pt.y, &tab);
	if (tab && tab->mode == TABMODE_LABEL) {
		return;
	}
	if (tab && tabindex != psy_INDEX_INVALID && (tabindex != self->selected || tab->istoggle)) {
		if (!tab->istoggle) {
			self->selected = tabindex;
		} else {
			if (tab->checkstate != TABCHECKSTATE_OFF) {
				tab->checkstate = TABCHECKSTATE_OFF;
			} else {
				tab->checkstate = TABCHECKSTATE_ON;
			}
		}
		psy_ui_component_invalidate(tabbar_base(self));
		psy_ui_component_update(tabbar_base(self));
		psy_signal_emit(&self->signal_change, self, 1, tabindex);
	}
}

uintptr_t tabbar_tabhittest(TabBar* self, double x, double y, Tab** rvtab)
{
	psy_List* p;
	uintptr_t rv;
	intptr_t c;
	const psy_ui_TextMetric* tm;

	assert(self);

	rv = psy_INDEX_INVALID;
	tabbar_calctabpositions(self);
	tm = psy_ui_component_textmetric(tabbar_base(self));	
	for (c = 0, p = self->tabs; p != NULL; psy_list_next(&p), ++c) {
		Tab* tab;
		psy_ui_RealRectangle r;

		tab = (Tab*)psy_list_entry(p);
		r = tab_position(tab, tm);
		if (psy_ui_realrectangle_intersect(&r,
				psy_ui_realpoint_make(x, y))) {
			rv = c;
			*rvtab = tab;
			break;
		}
	}
	return rv;
}

void tabbar_onmousemove(TabBar* self, psy_ui_MouseEvent* ev)
{	
	uintptr_t tabindex;
	Tab* tab;

	assert(self);

	tab = NULL;
	tabindex = tabbar_tabhittest(self, ev->pt.x, ev->pt.y, &tab);
	if (tab && tab->mode == TABMODE_LABEL) {
		self->hoverindex = psy_INDEX_INVALID;
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
	assert(self);

	self->hover = TRUE;
	psy_ui_component_invalidate(tabbar_base(self));
}

void tabbar_onmouseleave(TabBar* self)
{
	assert(self);

	self->hover = FALSE;
	psy_ui_component_invalidate(tabbar_base(self));
}

void tabbar_select(TabBar* self, uintptr_t tabindex)
{
	Tab* tab;

	assert(self);

	self->selected = tabindex;
	tab = tabbar_tab(self, tabindex);
	if (tab) {
		tab->checkstate = TABCHECKSTATE_ON;
	}
	psy_ui_component_invalidate(tabbar_base(self));
	psy_ui_component_update(tabbar_base(self));
	psy_signal_emit(&self->signal_change, self, 1, self->selected);
}

Tab* tabbar_append(TabBar* self, const char* label)
{
	Tab* tab;

	assert(self);

	tab = (Tab*) malloc(sizeof(Tab));
	if (tab) {
		char* text;
		
		tab_init(tab, label, 0, &self->defaulttabmargin);
		if (self->tabs == 0) {
			if (self->tabalignment == psy_ui_ALIGN_TOP) {
				tab->margin.left = psy_ui_value_makepx(10);
			} else {
				// tab->margin.top = psy_ui_value_makepx(0);
			}			
		}
		psy_list_append(&self->tabs, tab);
		if (tab->translation) {
			text = tab->translation;
		} else {
			text = tab->text;
		}
		tab->size = psy_ui_component_textsize(tabbar_base(self), text);
		tab->size.height = self->defaulttabheight;
	}
	return tab;
}

void tabbar_append_tabs(TabBar* self, const char* label, ...)
{
	const char* currlabel;
	va_list ap;

	assert(self);
	
	va_start(ap, label);
	for (currlabel = label; currlabel != NULL; currlabel = va_arg(ap, const char*)) {
		tabbar_append(self, currlabel);		
	}
	va_end(ap);
}

void tabbar_clear(TabBar* self)
{
	assert(self);

	self->selected = 0;
	psy_list_deallocate(&self->tabs, (psy_fp_disposefunc)tab_dispose);	
}

void tabbar_rename_tabs(TabBar* self, const char* label, ...)
{
	const char* currlabel;
	psy_List* t;
	va_list ap;

	assert(self);

	va_start(ap, label);
	for (t = self->tabs, currlabel = label; t != NULL && currlabel != NULL;
			t = t->next, currlabel = va_arg(ap, const char*)) {
		Tab* tab;

		tab = (Tab*)t->entry;
		tab_settext(tab, currlabel);
	}
	va_end(ap);
}

void tabbar_settabalignment(TabBar* self, psy_ui_AlignType alignment)
{
	assert(self);

	self->tabalignment = alignment;
}

void tabbar_settabmargin(TabBar* self, uintptr_t tabindex,
	const psy_ui_Margin* margin)
{
	Tab* tab;

	assert(self);

	tab = tabbar_tab(self, tabindex);
	if (tab) {
		if (margin) {
			tab->margin = *margin;
		} else {
			tab->margin = self->defaulttabmargin;
		}
	}		
}

void tabbar_settabmode(TabBar* self, uintptr_t tabindex, TabMode mode)
{
	Tab* tab;

	assert(self);

	tab = tabbar_tab(self, tabindex);
	if (tab) {
		tab->mode = mode;
		tab->istoggle = (mode == TABMODE_MULTISEL);
	}
}

void tabbar_onalign(TabBar* self)
{
	psy_List* p;

	assert(self);
	
	for (p = self->tabs; p != NULL; psy_list_next(&p)) {
		Tab* tab;
		char* text;

		tab = (Tab*)p->entry;
		if (tab->translation) {
			text = tab->translation;
		} else {
			text = tab->text;
		}
		tab->size = psy_ui_size_make(
			psy_ui_component_textsize(tabbar_base(self), text).width,
			self->defaulttabheight);
	}		
}

void tabbar_onpreferredsize(TabBar* self, const psy_ui_Size* limit, psy_ui_Size* rv)
{	
	psy_ui_RealSize maxsize;

	assert(self);
		
	maxsize = tabbar_calctabpositions(self);
	rv->width = psy_ui_value_makepx(maxsize.width);
	rv->height = psy_ui_value_makepx(maxsize.height);	
}

Tab* tabbar_tab(TabBar* self, uintptr_t tabindex)
{
	Tab* rv = 0;
	psy_List* p;	
	
	assert(self);

	p = psy_list_at(self->tabs, tabindex);
	if (p) {		
		rv = (Tab*)psy_list_entry(p);
	}
	return rv;
}

const Tab* tabbar_tab_const(const TabBar* self, uintptr_t tabindex)
{
	assert(self);

	return tabbar_tab((TabBar*)self, tabindex);
}

void tabbar_setdefaulttabmargin(TabBar* self, const psy_ui_Margin* margin)
{
	psy_List* tabs;

	assert(self);

	self->defaulttabmargin = *margin;	
	for (tabs = self->tabs; tabs != NULL; psy_list_next(&tabs)) {
		Tab* tab;

		tab = (Tab*)psy_list_entry(tabs);
		if (tab != NULL) {
			tab->margin = self->defaulttabmargin;
		}
	}
}

TabCheckState tabbar_checkstate(const TabBar* self, uintptr_t tabindex)
{	
	const Tab* tab;

	assert(self);

	tab = tabbar_tab_const(self, tabindex);
	if (tab) {
		return tab->checkstate;		
	}
	return TABCHECKSTATE_OFF;
}

uintptr_t tabbar_numchecked(const TabBar* self)
{
	uintptr_t rv;
	const psy_List* tabs;
	
	assert(self);

	for (rv = 0, tabs = self->tabs; tabs != NULL; psy_list_next_const(&tabs)) {
		const Tab* tab;

		tab = (Tab*)psy_list_entry_const(tabs);
		if (tab != NULL && tab->checkstate != TABCHECKSTATE_OFF) {
			++rv;
		}		
	}
	return rv;
}

void tabbar_resettabcheckstates(TabBar* self)
{
	psy_List* tabs;

	assert(self);

	for (tabs = self->tabs; tabs != NULL; psy_list_next(&tabs)) {
		Tab* tab;

		tab = (Tab*)psy_list_entry(tabs);
		if (tab != NULL) {
			tab->checkstate = TABCHECKSTATE_OFF;
		}
	}
}

void tabbar_onlanguagechanged(TabBar* self)
{	
	psy_List* tabs;

	assert(self);

	for (tabs = self->tabs; tabs != NULL; psy_list_next(&tabs)) {
		Tab* tab;

		tab = (Tab*)psy_list_entry(tabs);
		if (tab != NULL) {
			psy_strreset(&tab->translation, psy_ui_translate(tab->text));
		}
	}
	psy_ui_component_invalidate(&self->component);
}

void tabbar_onupdatestyles(TabBar* self)
{
	psy_ui_component_setstyletypes(&self->component,
		psy_ui_STYLE_TABBAR,
		psy_ui_STYLE_TABBAR_HOVER,
		psy_ui_STYLE_TABBAR_SELECT);
	psy_ui_style_copy(&self->style_tab, psy_ui_style(psy_ui_STYLE_TAB));
	psy_ui_style_copy(&self->style_tab_hover, psy_ui_style(psy_ui_STYLE_TAB_HOVER));
	psy_ui_style_copy(&self->style_tab_select, psy_ui_style(psy_ui_STYLE_TAB_SELECT));
	psy_ui_style_copy(&self->style_tab_label, psy_ui_style(psy_ui_STYLE_COMMON));	
}
