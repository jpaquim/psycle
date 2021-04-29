// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uitabbar.h"

// platform
#include "../../detail/portable.h"

// psy_ui_Tab
// protoypes
static void psy_ui_tab_ondestroy(psy_ui_Tab*);
static void psy_ui_tab_onmousedown(psy_ui_Tab*, psy_ui_MouseEvent*);
static void psy_ui_tab_ondraw(psy_ui_Tab*, psy_ui_Graphics*);
static void psy_ui_tab_onpreferredsize(psy_ui_Tab*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_tab_onlanguagechanged(psy_ui_Tab*);
// vtable
static psy_ui_ComponentVtable psy_ui_tab_vtable;
static bool psy_ui_tab_vtable_initialized = FALSE;

static void psy_ui_tab_vtable_init(psy_ui_Tab* self)
{
	if (!psy_ui_tab_vtable_initialized) {
		psy_ui_tab_vtable = *(self->component.vtable);
		psy_ui_tab_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			psy_ui_tab_ondestroy;
		psy_ui_tab_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			psy_ui_tab_onmousedown;
		psy_ui_tab_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_tab_ondraw;
		psy_ui_tab_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			psy_ui_tab_onpreferredsize;
		psy_ui_tab_vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			psy_ui_tab_onlanguagechanged;
		psy_ui_tab_vtable_initialized = TRUE;
	}
	self->component.vtable = &psy_ui_tab_vtable;
}
// implementation
void psy_ui_tab_init(psy_ui_Tab* self, psy_ui_Component* parent,
	psy_ui_Component* view, const char* text, uintptr_t index)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, view);
	psy_ui_tab_vtable_init(self);
	psy_ui_component_setstyletypes(&self->component,
		psy_ui_STYLE_TAB, psy_ui_STYLE_TAB_HOVER, psy_ui_STYLE_TAB_SELECT,
		psy_INDEX_INVALID);
	psy_signal_init(&self->signal_clicked);
	psy_ui_bitmap_init(&self->icon);
	self->text = psy_strdup(text);
	self->translation = NULL;
	self->preventtranslation = FALSE;
	psy_strreset(&self->translation, psy_ui_translate(text));
	self->istoggle = FALSE;
	self->mode = psy_ui_TABMODE_SINGLESEL;
	self->checkstate = 0;	
	self->index = index;	
	self->bitmapident = 1.0;	
}

psy_ui_Tab* psy_ui_tab_alloc(void)
{
	return (psy_ui_Tab*)malloc(sizeof(psy_ui_Tab));
}

psy_ui_Tab* psy_ui_tab_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view, const char* text, uintptr_t index)
{
	psy_ui_Tab* rv;

	rv = psy_ui_tab_alloc();
	if (rv) {
		psy_ui_tab_init(rv, parent, view, text, index);
		psy_ui_component_deallocateafterdestroyed(&rv->component);
	}
	return rv;
}

void psy_ui_tab_ondestroy(psy_ui_Tab* self)
{
	assert(self);

	free(self->text);
	self->text = NULL;
	free(self->translation);
	self->translation = NULL;
	psy_ui_bitmap_dispose(&self->icon);
	psy_signal_dispose(&self->signal_clicked);
}

void psy_ui_tab_settext(psy_ui_Tab* self, const char* text)
{
	assert(self);

	psy_strreset(&self->text, text);	
	if (self->preventtranslation) {
		free(self->translation);
		self->translation = NULL;
	} else {
		psy_strreset(&self->translation, psy_ui_translate(text));
	}
}

void psy_ui_tab_setmode(psy_ui_Tab* self, TabMode mode)
{
	self->mode = mode;	
	if (mode == psy_ui_TABMODE_LABEL) {
		psy_ui_component_setstyletypes(&self->component,
			psy_ui_STYLE_LABEL, psy_INDEX_INVALID, psy_INDEX_INVALID,
			psy_INDEX_INVALID);
	} else {
		psy_ui_component_setstyletypes(&self->component,
			psy_ui_STYLE_TAB, psy_ui_STYLE_TAB_HOVER, psy_ui_STYLE_TAB_SELECT,
			psy_INDEX_INVALID);
	}
}

void psy_ui_tab_preventtranslation(psy_ui_Tab* self)
{
	self->preventtranslation = TRUE;
	free(self->translation);
	self->translation = NULL;
}

void psy_ui_tab_ondraw(psy_ui_Tab* self, psy_ui_Graphics* g)
{
	char* text;
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;	
	double centery;
	double textident;

	assert(self);	
		
	textident = 0.0;
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_size_px(&self->component);	
	if (!psy_ui_bitmap_empty(&self->icon)) {
		psy_ui_RealSize bpmsize;
		double vcenter;

		bpmsize = psy_ui_bitmap_size(&self->icon);
		vcenter = (size.height - bpmsize.height) / 2.0;
		psy_ui_drawbitmap(g, &self->icon,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(0.0, vcenter),
				bpmsize),
			psy_ui_realpoint_zero());
		textident += bpmsize.width + tm->tmAveCharWidth * self->bitmapident;
	}
	if (self->translation) {
		text = self->translation;
	} else {
		text = self->text;
	}
	if (psy_strlen(text) > 0) {
		centery = (size.height - tm->tmHeight) / 2.0;
		psy_ui_textout(g, textident, centery, text, psy_strlen(text));
	}
}

void psy_ui_tab_onpreferredsize(psy_ui_Tab* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	psy_ui_Margin spacing;
	const psy_ui_TextMetric* tm;
	char* text;	
	
	if (self->translation) {
		text = self->translation;
	} else {
		text = self->text;
	}	
	*rv = psy_ui_component_textsize(&self->component, text);
	rv->height = psy_ui_value_make_eh(1.8);
	tm = psy_ui_component_textmetric(psy_ui_tab_base(self));
	if (!psy_ui_bitmap_empty(&self->icon)) {
		psy_ui_RealSize bpmsize;				
		psy_ui_RealSize textsizepx;
		
		bpmsize = psy_ui_bitmap_size(&self->icon);
		textsizepx = psy_ui_size_px(rv, tm);				
		rv->width = psy_ui_value_make_px(textsizepx.width + bpmsize.width
			+ tm->tmAveCharWidth * self->bitmapident);
	}	
	spacing = psy_ui_component_spacing(psy_ui_tab_base(self));
	rv->height = psy_ui_add_values(rv->height, psy_ui_margin_height(&spacing, tm), tm);
	rv->width = psy_ui_add_values(rv->width, psy_ui_margin_width(&spacing, tm), tm);
}

void psy_ui_tab_onmousedown(psy_ui_Tab* self, psy_ui_MouseEvent* ev)
{
	if (self->mode != psy_ui_TABMODE_LABEL) {
		if (self->istoggle) {
			if (self->checkstate != TABCHECKSTATE_OFF) {
				self->checkstate = TABCHECKSTATE_OFF;
				psy_ui_component_removestylestate(&self->component,
					psy_ui_STYLE_TAB_SELECT);
			} else {
				self->checkstate = TABCHECKSTATE_ON;
				psy_ui_component_addstylestate(&self->component,
					psy_ui_STYLE_TAB_SELECT);
			}
		}		
		psy_signal_emit(&self->signal_clicked, self, 0);
	}
}

void psy_ui_tab_onlanguagechanged(psy_ui_Tab* self)
{	
	psy_strreset(&self->translation, psy_ui_translate(self->text));
	psy_ui_component_invalidate(&self->component);
}

// psy_ui_TabBar
// prototypes
static void tabbar_ondestroy(psy_ui_TabBar*);
static void tabbar_build(psy_ui_TabBar*);
static void tabbar_ontabclicked(psy_ui_TabBar*, psy_ui_Tab* sender);
static void tabbar_onmousewheel(psy_ui_TabBar*, psy_ui_MouseEvent*);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_TabBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			tabbar_ondestroy;
		vtable.onmousewheel =
			(psy_ui_fp_component_onmouseevent)
			tabbar_onmousewheel;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}
// implementation
void psy_ui_tabbar_init(psy_ui_TabBar* self, psy_ui_Component* parent)
{
	assert(self);

	psy_ui_component_init(psy_ui_tabbar_base(self), parent, NULL);
	vtable_init(self);
	psy_ui_component_doublebuffer(psy_ui_tabbar_base(self));	
	psy_ui_component_setstyletype(&self->component, psy_ui_STYLE_TABBAR);
	psy_signal_init(&self->signal_change);
	self->numtabs = 0;
	self->selected = 0;
	self->preventtranslation = FALSE;
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_zero());
}

void tabbar_ondestroy(psy_ui_TabBar* self)
{	
	assert(self);
	
	psy_signal_dispose(&self->signal_change);
}

void psy_ui_tabbar_settabalign(psy_ui_TabBar* self, psy_ui_AlignType align)
{	
	psy_List* p;
	psy_List* q;

	assert(self);
	
	psy_ui_component_setdefaultalign(&self->component, align,
		self->component.containeralign->insertmargin);
	q = psy_ui_component_children(psy_ui_tabbar_base(self),
		psy_ui_NONRECURSIVE);
	for (p = q; p != NULL; p = p->next) {
		psy_ui_component_setalign((psy_ui_Component*)psy_list_entry(p),
			align);
	}
	psy_list_free(q);	
}

void psy_ui_tabbar_preventtranslation(psy_ui_TabBar* self)
{
	self->preventtranslation = TRUE;
}

void tabbar_ontabclicked(psy_ui_TabBar* self, psy_ui_Tab* sender)
{	
	psy_ui_tabbar_select(self, sender->index);	
}

void psy_ui_tabbar_mark(psy_ui_TabBar* self, uintptr_t tabindex)
{
	psy_ui_Tab* tab;

	assert(self);

	tab = psy_ui_tabbar_tab(self, tabindex);
	if (tab) {
		if (tab && tab->mode == psy_ui_TABMODE_LABEL) {
			return;
		}
		if (!tab->istoggle) {
			psy_ui_Tab* oldtab;

			oldtab = psy_ui_tabbar_tab(self, self->selected);
			if (oldtab) {
				psy_ui_component_removestylestate(&oldtab->component,
					psy_ui_STYLESTATE_SELECT);
			}
			self->selected = tabindex;
			psy_ui_component_addstylestate(&tab->component,
				psy_ui_STYLESTATE_SELECT);
		}
	}
}

void psy_ui_tabbar_unmark(psy_ui_TabBar* self)
{
	psy_ui_component_removestylestate_children(&self->component,
		psy_ui_STYLESTATE_SELECT);
}

void psy_ui_tabbar_select(psy_ui_TabBar* self, uintptr_t tabindex)
{
	psy_ui_tabbar_mark(self, tabindex);
	psy_signal_emit(&self->signal_change, self, 1, tabindex);	
}

psy_ui_Tab* psy_ui_tabbar_append(psy_ui_TabBar* self, const char* label)
{
	psy_ui_Tab* tab;

	assert(self);

	tab = psy_ui_tab_allocinit(&self->component, &self->component,
		label, self->numtabs);
	if (self->preventtranslation) {
		psy_ui_tab_preventtranslation(tab);
	}
	++self->numtabs;
	psy_signal_connect(&tab->signal_clicked, self,
		tabbar_ontabclicked);	
	return tab;
}

void psy_ui_tabbar_append_tabs(psy_ui_TabBar* self, const char* label, ...)
{
	const char* curr;
	va_list ap;

	assert(self);
	
	va_start(ap, label);
	for (curr = label; curr != NULL; curr = va_arg(ap, const char*)) {
		psy_ui_tabbar_append(self, curr);
	}
	va_end(ap);
}

void psy_ui_tabbar_clear(psy_ui_TabBar* self)
{
	assert(self);

	self->selected = 0;	
	psy_ui_component_clear(&self->component);
	self->numtabs = 0;
}

void psy_ui_tabbar_settabmode(psy_ui_TabBar* self, uintptr_t tabindex,
	TabMode mode)
{
	psy_ui_Tab* tab;

	assert(self);

	tab = psy_ui_tabbar_tab(self, tabindex);
	if (tab) {
		psy_ui_tab_setmode(tab, mode);
	}
}

psy_ui_Tab* psy_ui_tabbar_tab(psy_ui_TabBar* self, uintptr_t tabindex)
{	
	assert(self);

	return (psy_ui_Tab*)psy_ui_component_at(&self->component, tabindex);	
}

const psy_ui_Tab* psy_ui_tabbar_tab_const(const psy_ui_TabBar* self,
	uintptr_t tabindex)
{
	assert(self);

	return psy_ui_tabbar_tab((psy_ui_TabBar*)self, tabindex);
}

void tabbar_onmousewheel(psy_ui_TabBar* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (ev->delta > 0) {
		if (self->selected + 1 < self->numtabs) {
			psy_ui_tabbar_select(self, self->selected + 1);
		}
	} else if (ev->delta < 0) {
		if (self->selected > 0) {
			psy_ui_tabbar_select(self, self->selected - 1);
		}
	}		
	psy_ui_mouseevent_preventdefault(ev);
}
