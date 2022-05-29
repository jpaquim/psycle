/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uitabbar.h"
/* local */
#include "uiapp.h"
/* platform */
#include "../../detail/portable.h"

/* psy_ui_Tab */
/* protoypes */
static void psy_ui_tab_on_destroy(psy_ui_Tab*);
static void psy_ui_tab_on_mouse_down(psy_ui_Tab*, psy_ui_MouseEvent*);
static void psy_ui_tab_ondraw(psy_ui_Tab*, psy_ui_Graphics*);
static void psy_ui_tab_onpreferredsize(psy_ui_Tab*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_tab_onlanguagechanged(psy_ui_Tab*);
static void psy_ui_tab_onupdatestyles(psy_ui_Tab*);
static void psy_ui_tab_loadbitmaps(psy_ui_Tab*);
/* vtable */
static psy_ui_ComponentVtable psy_ui_tab_vtable;
static bool psy_ui_tab_vtable_initialized = FALSE;

static void psy_ui_tab_vtable_init(psy_ui_Tab* self)
{
	if (!psy_ui_tab_vtable_initialized) {
		psy_ui_tab_vtable = *(self->component.vtable);
		psy_ui_tab_vtable.on_destroy =
			(psy_ui_fp_component_event)
			psy_ui_tab_on_destroy;
		psy_ui_tab_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_tab_on_mouse_down;
		psy_ui_tab_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_tab_ondraw;
		psy_ui_tab_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			psy_ui_tab_onpreferredsize;
		psy_ui_tab_vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			psy_ui_tab_onlanguagechanged;
		psy_ui_tab_vtable.onupdatestyles =
			(psy_ui_fp_component_event)
			psy_ui_tab_onupdatestyles;
		psy_ui_tab_vtable_initialized = TRUE;
	}
	self->component.vtable = &psy_ui_tab_vtable;
}
/* implementation */
void psy_ui_tab_init(psy_ui_Tab* self, psy_ui_Component* parent,
	const char* text, uintptr_t index)
{
	assert(self);
	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_tab_vtable_init(self);
	psy_ui_component_set_style_types(&self->component,
		psy_ui_STYLE_TAB, psy_ui_STYLE_TAB_HOVER, psy_ui_STYLE_TAB_SELECT,
		psy_INDEX_INVALID);
	psy_signal_init(&self->signal_clicked);
	psy_ui_bitmap_init(&self->bitmapicon);
	self->text = psy_strdup(text);
	self->translation = NULL;
	self->prevent_translation = FALSE;
	psy_strreset(&self->translation, psy_ui_translate(text));
	self->istoggle = FALSE;
	self->mode = psy_ui_TABMODE_SINGLESEL;
	self->checkstate = 0;	
	self->index = index;
	self->target_id = psy_INDEX_INVALID;
	self->bitmapident = 1.0;
	self->lightresourceid = psy_INDEX_INVALID;
	self->darkresourceid = psy_INDEX_INVALID;
	psy_ui_colour_init(&self->bitmaptransparency);
}

psy_ui_Tab* psy_ui_tab_alloc(void)
{
	return (psy_ui_Tab*)malloc(sizeof(psy_ui_Tab));
}

psy_ui_Tab* psy_ui_tab_allocinit(psy_ui_Component* parent,
	const char* text, uintptr_t index)
{
	psy_ui_Tab* rv;

	rv = psy_ui_tab_alloc();
	if (rv) {
		psy_ui_tab_init(rv, parent, text, index);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void psy_ui_tab_on_destroy(psy_ui_Tab* self)
{
	assert(self);

	free(self->text);
	self->text = NULL;
	free(self->translation);
	self->translation = NULL;
	psy_ui_bitmap_dispose(&self->bitmapicon);
	psy_signal_dispose(&self->signal_clicked);
}

void psy_ui_tab_settext(psy_ui_Tab* self, const char* text)
{
	assert(self);

	psy_strreset(&self->text, text);	
	if (self->prevent_translation) {
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
		psy_ui_component_set_style_types(&self->component,
			psy_ui_STYLE_TAB_LABEL, psy_INDEX_INVALID, psy_INDEX_INVALID,
			psy_INDEX_INVALID);		
	} else {
		psy_ui_component_set_style_types(&self->component,
			psy_ui_STYLE_TAB, psy_ui_STYLE_TAB_HOVER, psy_ui_STYLE_TAB_SELECT,
			psy_INDEX_INVALID);
	}
}

void psy_ui_tab_preventtranslation(psy_ui_Tab* self)
{
	self->prevent_translation = TRUE;
	free(self->translation);
	self->translation = NULL;
}

void psy_ui_tab_load_resource(psy_ui_Tab* self,
	uintptr_t lightresourceid, uintptr_t darkresourceid,
	psy_ui_Colour transparency)
{
	self->lightresourceid = lightresourceid;
	self->darkresourceid = darkresourceid;
	self->bitmaptransparency = transparency;
	psy_ui_tab_loadbitmaps(self);
}

void psy_ui_tab_ondraw(psy_ui_Tab* self, psy_ui_Graphics* g)
{
	char* text;
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;
	psy_ui_RealPoint center;	
	double textident;

	assert(self);	
		
	textident = 0.0;
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_scroll_size_px(&self->component);
	center = psy_ui_realpoint_make(textident, 0.0);
	if (!psy_ui_bitmap_empty(&self->bitmapicon)) {
		psy_ui_RealSize bpmsize;
		double vcenter;

		bpmsize = psy_ui_bitmap_size(&self->bitmapicon);
		vcenter = (size.height - bpmsize.height) / 2.0;
		psy_ui_drawbitmap(g, &self->bitmapicon,
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
		center.x = textident;
		center.y = (size.height - tm->tmHeight) / 2.0;
		psy_ui_textout(g, center, text, psy_strlen(text));
	}
}

void psy_ui_tab_onpreferredsize(psy_ui_Tab* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{		
	const psy_ui_TextMetric* tm;
	char* text;	
	const psy_ui_Font* font;	
	
	if (self->translation) {
		text = self->translation;
	} else {
		text = self->text;
	}
	font = psy_ui_component_font(&self->component);
	if (font) {
		*rv = psy_ui_font_textsize(font, text, psy_strlen(text));
	} else {
		*rv = psy_ui_size_zero();
	}	
	rv->height = psy_ui_value_make_eh(1.8);
	tm = psy_ui_component_textmetric(psy_ui_tab_base(self));
	if (!psy_ui_bitmap_empty(&self->bitmapicon)) {
		psy_ui_RealSize bpmsize;				
		psy_ui_RealSize textsizepx;
		
		bpmsize = psy_ui_bitmap_size(&self->bitmapicon);
		textsizepx = psy_ui_size_px(rv, tm, NULL);
		rv->width = psy_ui_value_make_px(textsizepx.width + bpmsize.width
			+ tm->tmAveCharWidth * self->bitmapident);
	}	
}

void psy_ui_tab_on_mouse_down(psy_ui_Tab* self, psy_ui_MouseEvent* ev)
{	
	if (self->mode != psy_ui_TABMODE_LABEL) {
		if (self->istoggle) {
			if (self->checkstate != TABCHECKSTATE_OFF) {
				self->checkstate = TABCHECKSTATE_OFF;
				psy_ui_component_remove_style_state(&self->component,
					psy_ui_STYLE_TAB_SELECT);
			} else {
				self->checkstate = TABCHECKSTATE_ON;
				psy_ui_component_add_style_state(&self->component,
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

void psy_ui_tab_onupdatestyles(psy_ui_Tab* self)
{
	psy_ui_tab_loadbitmaps(self);
}

void psy_ui_tab_loadbitmaps(psy_ui_Tab* self)
{
	if (psy_ui_app_hasdarktheme(psy_ui_app())) {
		if (self->darkresourceid != psy_INDEX_INVALID) {
			psy_ui_bitmap_loadresource(&self->bitmapicon, self->darkresourceid);
		}
	} else {
		if (self->lightresourceid != psy_INDEX_INVALID) {
			psy_ui_bitmap_loadresource(&self->bitmapicon, self->lightresourceid);
		}
	}
	if (!self->bitmaptransparency.mode.transparent) {
		psy_ui_bitmap_settransparency(&self->bitmapicon,
			self->bitmaptransparency);
	}
}

/* psy_ui_TabBar */
/* prototypes */
static void tabbar_on_destroy(psy_ui_TabBar*);
static void tabbar_on_tab_clicked(psy_ui_TabBar*, psy_ui_Tab* sender);
static void tabbar_onmousewheel(psy_ui_TabBar*, psy_ui_MouseEvent*);
/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_TabBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroy =
			(psy_ui_fp_component_event)
			tabbar_on_destroy;
		vtable.onmousewheel =
			(psy_ui_fp_component_on_mouse_event)
			tabbar_onmousewheel;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}

/* implementation */
void psy_ui_tabbar_init(psy_ui_TabBar* self, psy_ui_Component* parent)
{
	assert(self);

	psy_ui_component_init(psy_ui_tabbar_base(self), parent, NULL);
	vtable_init(self);	
	psy_ui_component_set_style_type(&self->component, psy_ui_STYLE_TABBAR);
	psy_signal_init(&self->signal_change);
	self->num_tabs = 0;
	self->selected = 0;
	self->prevent_translation = FALSE;
	self->tab_alignment = psy_ui_ALIGN_LEFT;
	psy_ui_component_set_defaultalign(&self->component, self->tab_alignment,
		psy_ui_margin_zero());	
}

void tabbar_on_destroy(psy_ui_TabBar* self)
{	
	assert(self);
	
	psy_signal_dispose(&self->signal_change);
}

void psy_ui_tabbar_settabalign(psy_ui_TabBar* self, psy_ui_AlignType align)
{	
	psy_List* p;
	psy_List* q;

	assert(self);
	
	self->tab_alignment = align;
	psy_ui_component_set_defaultalign(&self->component, align,
		self->component.containeralign->insertmargin);
	q = psy_ui_component_children(psy_ui_tabbar_base(self),
		psy_ui_NONE_RECURSIVE);
	for (p = q; p != NULL; p = p->next) {
		psy_ui_component_set_align((psy_ui_Component*)psy_list_entry(p),
			align);
	}
	psy_list_free(q);	
}

void psy_ui_tabbar_preventtranslation(psy_ui_TabBar* self)
{
	self->prevent_translation = TRUE;
}

void tabbar_on_tab_clicked(psy_ui_TabBar* self, psy_ui_Tab* sender)
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
				psy_ui_component_remove_style_state(&oldtab->component,
					psy_ui_STYLESTATE_SELECT);
			}
			self->selected = tabindex;
			psy_ui_component_add_style_state(&tab->component,
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

psy_ui_Tab* psy_ui_tabbar_append(psy_ui_TabBar* self, const char* label,
	uintptr_t target_id, uintptr_t lightresourceid, uintptr_t darkresourceid,
	psy_ui_Colour transparency)
{
	psy_ui_Tab* rv;

	assert(self);

	rv = psy_ui_tab_allocinit(&self->component, label, self->num_tabs);
	if (rv) {
		psy_ui_tab_set_target_id(rv, target_id);
		if (self->prevent_translation) {
			psy_ui_tab_preventtranslation(rv);
		}
		psy_ui_tab_load_resource(rv, lightresourceid, darkresourceid,
			transparency);
		++self->num_tabs;
		psy_signal_connect(&rv->signal_clicked, self, tabbar_on_tab_clicked);
	}
	return rv;
}

void psy_ui_tabbar_append_tabs(psy_ui_TabBar* self, const char* label, ...)
{
	const char* curr;
	va_list ap;

	assert(self);
	
	va_start(ap, label);
	for (curr = label; curr != NULL; curr = va_arg(ap, const char*)) {
		psy_ui_tabbar_append(self, curr, psy_INDEX_INVALID, psy_INDEX_INVALID,
			psy_INDEX_INVALID, psy_ui_colour_white());
	}
	va_end(ap);
}

void psy_ui_tabbar_clear(psy_ui_TabBar* self)
{
	assert(self);

	self->selected = 0;	
	psy_ui_component_clear(&self->component);
	self->num_tabs = 0;
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
	intptr_t delta;

	assert(self);

	delta = psy_ui_mouseevent_delta(ev);
	if (self->tab_alignment == psy_ui_ALIGN_TOP ||
		self->tab_alignment == psy_ui_ALIGN_BOTTOM) {
		delta *= -1;
	}
	if (delta > 0) {
		if (self->selected + 1 < self->num_tabs) {
			psy_ui_tabbar_select(self, self->selected + 1);
		}
	} else if (delta < 0) {
		if (self->selected > 0) {
			psy_ui_tabbar_select(self, self->selected - 1);
		}
	}		
	psy_ui_mouseevent_prevent_default(ev);
}
