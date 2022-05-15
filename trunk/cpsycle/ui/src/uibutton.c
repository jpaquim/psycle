/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uibutton.h"
/* local */
#include "uiapp.h"
/* platform */
#include "../../detail/trace.h"
#include "../../detail/portable.h"

/* prototypes */
static void psy_ui_button_on_destroy(psy_ui_Button*);
static void psy_ui_button_onlanguagechanged(psy_ui_Button*);
static void psy_ui_button_ondraw(psy_ui_Button*, psy_ui_Graphics*);
static void psy_ui_button_on_mouse_down(psy_ui_Button*, psy_ui_MouseEvent*);
static void psy_ui_button_on_mouse_up(psy_ui_Button*, psy_ui_MouseEvent*);
static void psy_ui_button_onpreferredsize(psy_ui_Button*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static void button_on_key_down(psy_ui_Button*, psy_ui_KeyboardEvent*);
static double psy_ui_button_width(psy_ui_Button*);
static void psy_ui_button_onupdatestyles(psy_ui_Button*);
static void psy_ui_button_loadbitmaps(psy_ui_Button*);
/* vtable */
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_Button* self)
{
	if (!vtable_initialized) {
		vtable = *(psy_ui_button_base(self)->vtable);
		super_vtable = *(psy_ui_button_base(self)->vtable);
		vtable.on_destroy =
			(psy_ui_fp_component_event)
			psy_ui_button_on_destroy;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_button_ondraw;
		vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			psy_ui_button_onpreferredsize;
		vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_button_on_mouse_down;
		vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_button_on_mouse_up;
		vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			button_on_key_down;
		vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			psy_ui_button_onlanguagechanged;
		vtable.onupdatestyles =
			(psy_ui_fp_component_event)
			psy_ui_button_onupdatestyles;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(psy_ui_button_base(self), &vtable);
}

/* implementation */
void psy_ui_button_init(psy_ui_Button* self, psy_ui_Component* parent)
{
	psy_ui_component_init(psy_ui_button_base(self), parent, NULL);
	vtable_init(self);	
	self->icon = psy_ui_ICON_NONE;
	self->charnumber = 0.0;
	self->linespacing = 1.0;
	self->bitmapident = 1.0;
	self->data = psy_INDEX_INVALID;
	self->textalignment = psy_ui_ALIGNMENT_CENTER;
	self->text = NULL;
	self->translation = NULL;
	self->translate = TRUE;	
	self->shiftstate = FALSE;
	self->ctrlstate = FALSE;
	self->buttonstate = 1;
	self->allowrightclick = FALSE;	
	self->stoppropagation = TRUE;
	self->lightresourceid = psy_INDEX_INVALID;
	self->darkresourceid = psy_INDEX_INVALID;
	psy_ui_colour_init(&self->bitmaptransparency);
	psy_ui_bitmap_init(&self->bitmapicon);
	psy_signal_init(&self->signal_clicked);	
	psy_ui_component_set_style_types(psy_ui_button_base(self),
		psy_ui_STYLE_BUTTON, psy_ui_STYLE_BUTTON_HOVER,
		psy_ui_STYLE_BUTTON_SELECT, psy_INDEX_INVALID);
	psy_ui_component_set_style_type_active(psy_ui_button_base(self),
		psy_ui_STYLE_BUTTON_ACTIVE);
	psy_ui_component_set_style_type_focus(psy_ui_button_base(self),
		psy_ui_STYLE_BUTTON_FOCUS);
}

void psy_ui_button_init_text(psy_ui_Button* self, psy_ui_Component* parent,
	const char* text)
{
	assert(self);

	psy_ui_button_init(self, parent);
	psy_ui_button_set_text(self, text);
}

void psy_ui_button_init_connect(psy_ui_Button* self, psy_ui_Component* parent,
	void* context, void* fp)
{
	assert(self);

	psy_ui_button_init(self, parent);
	psy_signal_connect(&self->signal_clicked, context, fp);
}

void psy_ui_button_init_text_connect(psy_ui_Button* self, psy_ui_Component*
	parent, const char* text, void* context, void* fp)
{
	assert(self);

	psy_ui_button_init_connect(self, parent, context, fp);
	psy_ui_button_set_text(self, text);	
}

psy_ui_Button* psy_ui_button_alloc(void)
{
	return (psy_ui_Button*)malloc(sizeof(psy_ui_Button));
}

psy_ui_Button* psy_ui_button_allocinit(psy_ui_Component* parent)
{
	psy_ui_Button* rv;

	rv = psy_ui_button_alloc();
	if (rv) {
		psy_ui_button_init(rv, parent);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void psy_ui_button_on_destroy(psy_ui_Button* self)
{	
	assert(self);

	free(self->text);
	self->text = NULL;
	free(self->translation);
	self->translation = NULL;
	psy_signal_dispose(&self->signal_clicked);
	psy_ui_bitmap_dispose(&self->bitmapicon);
}

void psy_ui_button_onlanguagechanged(psy_ui_Button* self)
{
	assert(self);
	
	if (self->translate) {		
		psy_strreset(&self->translation, psy_ui_translate(self->text));
		psy_ui_component_invalidate(&self->component);
	}
}

void psy_ui_button_ondraw(psy_ui_Button* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;	
	psy_ui_RealRectangle r;
	char* text;
	double cpx;
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(psy_ui_button_base(self));
	if (self->translate && self->translation) {
		text = self->translation;
	} else {
		text = self->text;
	}	
	size = psy_ui_component_size_px(psy_ui_button_base(self));
	r = psy_ui_realrectangle_make(psy_ui_realpoint_zero(), size);
	if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_HORIZONTAL) ==
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL) {
		cpx = (size.width - psy_ui_button_width(self)) / 2.0;
	} else {
		cpx = 0.0;
	}	
	psy_ui_button_width(self);
	if (!psy_ui_bitmap_empty(&self->bitmapicon)) {		
		psy_ui_RealSize srcbpmsize;
		psy_ui_RealSize destbpmsize;
		double vcenter;		
		double ratio;
						
		srcbpmsize = psy_ui_bitmap_size(&self->bitmapicon);		
		ratio = (tm->tmAscent - tm->tmDescent) / srcbpmsize.height;
		if (fabs(ratio - 1.0) < 0.15) {
			ratio = 1.0;
		}		
		destbpmsize.width = srcbpmsize.width * ratio;
		destbpmsize.height = srcbpmsize.height * ratio;
		vcenter = (size.height - destbpmsize.height) / 2.0;		
		
#if defined(DIVERSALIS__OS__MICROSOFT)		
		psy_ui_drawstretchedbitmap(g, &self->bitmapicon,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(cpx, vcenter),
			destbpmsize),
		psy_ui_realpoint_zero(),
		srcbpmsize);
#else
		psy_ui_drawfullbitmap(g, &self->bitmapicon,
			psy_ui_realpoint_zero());
#endif
		cpx += destbpmsize.width + tm->tmAveCharWidth * self->bitmapident;

	}
	if (self->icon != psy_ui_ICON_NONE) {
		psy_ui_IconDraw icondraw;
		double vcenter;

		if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_VERTICAL) ==
				psy_ui_ALIGNMENT_CENTER_VERTICAL) {
			vcenter = (size.height - psy_ui_buttonicon_size(self->icon).height)
				/ 2.0;
		} else {
			vcenter = 0.0;
		}		
		psy_ui_icondraw_init(&icondraw, self->icon);
		psy_ui_icondraw_draw(&icondraw, g,
			psy_ui_realpoint_make(cpx, vcenter),
			psy_ui_component_colour(&self->component));
		cpx += psy_ui_buttonicon_size(self->icon).width;
		cpx += tm->tmAveCharWidth * self->bitmapident;
	}
	if (psy_strlen(text) > 0) {				
		double vcenter;

		if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_VERTICAL) ==
			psy_ui_ALIGNMENT_CENTER_VERTICAL) {
			vcenter = (size.height - tm->tmHeight) / 2.0;
		} else {
			vcenter = 0.0;
		}
		if (psy_ui_component_inputprevented(&self->component)) {
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00777777));
		} else {
			psy_ui_settextcolour(g, psy_ui_component_colour(&self->component));
		}
		psy_ui_textoutrectangle(g, psy_ui_realpoint_make(cpx, vcenter),
			psy_ui_ETO_CLIPPED, r, text, strlen(text));
	}	
}

double psy_ui_button_width(psy_ui_Button* self)
{
	double rv;
	char* text;
	const psy_ui_TextMetric* tm;

	rv = 0.0;
	tm = psy_ui_component_textmetric(psy_ui_button_base(self));
	if (self->translate && self->translation) {
		text = self->translation;
	} else {
		text = self->text;
	}
	
	if (psy_strlen(text) > 0) {
		psy_ui_Size size;
		const psy_ui_Font* font;

		font = psy_ui_component_font(&self->component);
		if (font) {
			size = psy_ui_font_textsize(font, text, psy_strlen(text));
		} else {
			size = psy_ui_size_zero();
		}
		rv += psy_ui_value_px(&size.width, tm, NULL);
	}
	if (self->icon != psy_ui_ICON_NONE) {
		rv += psy_ui_buttonicon_size(self->icon).width;
		if (psy_strlen(text) > 0) {
			rv += tm->tmAveCharWidth * self->bitmapident;
		}
	}
	if (!psy_ui_bitmap_empty(&self->bitmapicon)) {
		psy_ui_RealSize srcbpmsize;		
		double ratio;

		srcbpmsize = psy_ui_bitmap_size(&self->bitmapicon);
		ratio = (tm->tmAscent - tm->tmDescent) / srcbpmsize.height;
		if (fabs(ratio - 1.0) < 0.15) {
			ratio = 1.0;
		}
		rv += srcbpmsize.width * ratio;	
		if (psy_strlen(text) > 0) {
			rv += tm->tmAveCharWidth * self->bitmapident;
		}
	}
	return rv;
}

void psy_ui_button_setcharnumber(psy_ui_Button* self, double number)
{
	self->charnumber = psy_max(0.0, number);
}

void psy_ui_button_setlinespacing(psy_ui_Button* self, double spacing)
{
	self->linespacing = spacing;
}

void psy_ui_button_onpreferredsize(psy_ui_Button* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{		
	const psy_ui_TextMetric* tm;			
	
	tm = psy_ui_component_textmetric(psy_ui_button_base(self));
	if (self->charnumber == 0) {		
		rv->width = psy_ui_value_make_px(psy_ui_button_width(self));
	} else {
		rv->width = psy_ui_value_make_ew(self->charnumber);
	}			
	rv->height = psy_ui_value_make_eh(self->linespacing);	
}

void psy_ui_button_on_mouse_down(psy_ui_Button* self, psy_ui_MouseEvent* ev)
{
	super_vtable.on_mouse_down(psy_ui_button_base(self), ev);
	if (!psy_ui_component_inputprevented(&self->component)) {
		psy_ui_component_capture(psy_ui_button_base(self));
	}
	if (self->stoppropagation) {
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void psy_ui_button_on_mouse_up(psy_ui_Button* self, psy_ui_MouseEvent* ev)
{	
	super_vtable.on_mouse_up(psy_ui_button_base(self), ev);
	if (!psy_ui_component_inputprevented(&self->component)) {
		psy_ui_component_release_capture(psy_ui_button_base(self));
		if (psy_ui_component_inputprevented(&self->component)) {
			psy_ui_mouseevent_stop_propagation(ev);
			return;
		}
		self->buttonstate = psy_ui_mouseevent_button(ev);
		if (self->allowrightclick || psy_ui_mouseevent_button(ev) == 1) {
			psy_ui_RealRectangle client_position;
						
			client_position = psy_ui_realrectangle_make(psy_ui_realpoint_zero(),
				psy_ui_component_scroll_size_px(psy_ui_button_base(self)));
			if (psy_ui_realrectangle_intersect(&client_position,
					psy_ui_mouseevent_offset(ev))) {
				self->shiftstate = psy_ui_mouseevent_shiftkey(ev);
				self->ctrlstate = psy_ui_mouseevent_ctrlkey(ev);
				psy_signal_emit(&self->signal_clicked, self, 0);
			}
			if (self->stoppropagation) {
				psy_ui_mouseevent_stop_propagation(ev);
			}
		}
	}
}

void psy_ui_button_set_text(psy_ui_Button* self, const char* text)
{	
	assert(self);

	psy_strreset(&self->text, text);
	if (self->translate) {
		psy_strreset(&self->translation, psy_ui_translate(text));
	}
	psy_ui_component_invalidate(psy_ui_button_base(self));
}

const char* psy_ui_button_text(const psy_ui_Button* self)
{
	return self->text;
}

void psy_ui_button_seticon(psy_ui_Button* self, psy_ui_ButtonIcon icon)
{
	self->icon = icon;
	psy_ui_component_invalidate(psy_ui_button_base(self));
}

void psy_ui_button_loadresource(psy_ui_Button* self,
	uintptr_t lightresourceid, uintptr_t darkresourceid,
	psy_ui_Colour transparency)
{
	self->lightresourceid = lightresourceid;
	self->darkresourceid = darkresourceid;
	self->bitmaptransparency = transparency;
	psy_ui_button_loadbitmaps(self);
}

void psy_ui_button_highlight(psy_ui_Button* self)
{
	if (!psy_ui_button_highlighted(self)) {		
		psy_ui_component_addstylestate(psy_ui_button_base(self),
			psy_ui_STYLESTATE_SELECT);		
	}
}

void psy_ui_button_disable_highlight(psy_ui_Button* self)
{
	if (psy_ui_button_highlighted(self)) {		
		psy_ui_component_removestylestate(psy_ui_button_base(self),
			psy_ui_STYLESTATE_SELECT);		
	}
}

bool psy_ui_button_highlighted(const psy_ui_Button* self)
{
	return (psy_ui_componentstyle_state(&self->component.style) &
		psy_ui_STYLESTATE_SELECT) == psy_ui_STYLESTATE_SELECT;
}

void psy_ui_button_settextcolour(psy_ui_Button* self, psy_ui_Colour colour)
{	
	psy_ui_component_setcolour(&self->component, colour);	
}

void psy_ui_button_settextalignment(psy_ui_Button* self,
	psy_ui_Alignment alignment)
{
	self->textalignment = alignment;
}

void psy_ui_button_prevent_translation(psy_ui_Button* self)
{
	self->translate = FALSE;
	if (self->translation) {
		free(self->translation);
		self->translation = NULL;
	}
}

void button_on_key_down(psy_ui_Button* self, psy_ui_KeyboardEvent* ev)
{
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_RETURN &&
			!psy_ui_component_inputprevented(&self->component)) {
		psy_signal_emit(&self->signal_clicked, self, 0);
		psy_ui_keyboardevent_stop_propagation(ev);
	}
}

void psy_ui_button_onupdatestyles(psy_ui_Button* self)
{
	psy_ui_button_loadbitmaps(self);
}

void psy_ui_button_loadbitmaps(psy_ui_Button* self)
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
