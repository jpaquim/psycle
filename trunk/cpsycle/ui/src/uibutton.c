// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uibutton.h"
// platform
#include "../../detail/trace.h"
#include "../../detail/portable.h"

// psy_ui_Button
// prototypes
static void ondestroy(psy_ui_Button*);
static void onlanguagechanged(psy_ui_Button*);
static void ondraw(psy_ui_Button*, psy_ui_Graphics*);
static void onmousedown(psy_ui_Button*, psy_ui_MouseEvent*);
static void onmouseup(psy_ui_Button*, psy_ui_MouseEvent*);
static void onmouseenter(psy_ui_Button*);
static void onmouseleave(psy_ui_Button*);
static void onpreferredsize(psy_ui_Button*, psy_ui_Size* limit, psy_ui_Size* rv);
static void enableinput(psy_ui_Button*);
static void preventinput(psy_ui_Button*);
static void button_onkeydown(psy_ui_Button*, psy_ui_KeyEvent*);
static void button_updatestyles(psy_ui_Button*);
static psy_ui_RealPoint psy_ui_button_center(psy_ui_Button*,
	psy_ui_RealPoint center, psy_ui_RealSize itemsize);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_Button* self)
{
	if (!vtable_initialized) {
		vtable = *(psy_ui_button_base(self)->vtable);
		vtable.ondestroy = (psy_ui_fp_component_ondestroy)ondestroy;
		vtable.enableinput = (psy_ui_fp_component_enableinput)enableinput;
		vtable.preventinput = (psy_ui_fp_component_preventinput)preventinput;
		vtable.ondraw = (psy_ui_fp_component_ondraw)ondraw;
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			onpreferredsize;
		vtable.onmousedown = (psy_ui_fp_component_onmouseevent)onmousedown;
		vtable.onmouseup = (psy_ui_fp_component_onmouseevent)onmouseup;
		vtable.onmouseenter = (psy_ui_fp_component_onmouseenter)onmouseenter;
		vtable.onmouseleave = (psy_ui_fp_component_onmouseleave)onmouseleave;		
		vtable.onkeydown = (psy_ui_fp_component_onkeyevent)button_onkeydown;
		vtable.onlanguagechanged = (psy_ui_fp_component_onlanguagechanged)
			onlanguagechanged;		
		vtable_initialized = TRUE;
	}
}
// implementation
void psy_ui_button_init(psy_ui_Button* self, psy_ui_Component* parent)
{
	psy_ui_component_init(psy_ui_button_base(self), parent);
	vtable_init(self);
	self->component.vtable = &vtable;	
	self->hover = 0;
	self->highlight = FALSE;
	self->icon = psy_ui_ICON_NONE;
	self->charnumber = 0;
	self->linespacing = 1.0;
	self->bitmapident = 1.0;
	self->textalignment = psy_ui_ALIGNMENT_CENTER_VERTICAL |
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL;	
	self->enabled = TRUE;
	self->text = NULL;
	self->translation = NULL;
	self->translate = TRUE;	
	self->shiftstate = FALSE;
	self->ctrlstate = FALSE;
	self->buttonstate = 1;
	self->allowrightclick = FALSE;	
	psy_ui_bitmap_init(&self->bitmapicon);
	psy_signal_init(&self->signal_clicked);	
	psy_ui_component_setstyletypes(psy_ui_button_base(self),
		psy_ui_STYLE_BUTTON, psy_ui_STYLE_BUTTON_HOVER,
		psy_ui_STYLE_BUTTON_SELECT);
}

void psy_ui_button_init_text(psy_ui_Button* self, psy_ui_Component* parent,
	const char* text)
{
	assert(self);

	psy_ui_button_init(self, parent);
	psy_ui_button_settext(self, text);
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
	psy_ui_button_settext(self, text);
}

void ondestroy(psy_ui_Button* self)
{	
	assert(self);

	free(self->text);
	self->text = NULL;
	free(self->translation);
	self->translation = NULL;
	psy_signal_dispose(&self->signal_clicked);
	psy_ui_bitmap_dispose(&self->bitmapicon);
}

void onlanguagechanged(psy_ui_Button* self)
{
	assert(self);

	psy_strreset(&self->translation, psy_ui_translate(self->text));	
}

void ondraw(psy_ui_Button* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;	
	psy_ui_RealRectangle r;
	char* text;
	double ident;
	
	if (self->translate && self->translation) {
		text = self->translation;
	} else {
		text = self->text;
	}
	size = psy_ui_component_sizepx(psy_ui_button_base(self));
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
	ident = 0.0;
	if (!psy_ui_bitmap_empty(&self->bitmapicon)) {
		psy_ui_RealSize srcbpmsize;
		psy_ui_RealSize destbpmsize;
		double vcenter;
		const psy_ui_TextMetric* tm;
		double ratio;

		tm = psy_ui_component_textmetric(psy_ui_button_base(self));
		srcbpmsize = psy_ui_bitmap_size(&self->bitmapicon);		
		ratio = (tm->tmAscent - tm->tmDescent) / srcbpmsize.height;
		if (fabs(ratio - 1.0) < 0.15) {
			ratio = 1.0;
		}		
		destbpmsize.width = srcbpmsize.width * ratio;
		destbpmsize.height = srcbpmsize.height * ratio;
		vcenter = (size.height - destbpmsize.height) / 2.0;
		psy_ui_drawstretchedbitmap(g, &self->bitmapicon,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(0, vcenter),
				destbpmsize),
			psy_ui_realpoint_zero(),
			srcbpmsize);
		ident += destbpmsize.width + tm->tmAveCharWidth * self->bitmapident;
	}
	if (self->icon != psy_ui_ICON_NONE) {
		psy_ui_IconDraw icondraw;
		const psy_ui_TextMetric* tm;
		psy_ui_RealSize buttonsize;

		buttonsize = psy_ui_component_sizepx(psy_ui_button_base(self));		
		psy_ui_icondraw_init(&icondraw, self->icon,
			self->component.style.currstyle);
		psy_ui_icondraw_draw(&icondraw, g,
			psy_ui_button_center(self,
				psy_ui_realpoint_make(12.0, 0.0),
				psy_ui_realsize_make(buttonsize.width, 8.0)));
		tm = psy_ui_component_textmetric(psy_ui_button_base(self));
		if (psy_strlen(text) > 0) {
			ident += tm->tmAveCharWidth * 2;
		}
	}
	if (psy_strlen(text) > 0) {
		psy_ui_Size textsize;		
		const psy_ui_TextMetric* tm;
		
		textsize = psy_ui_component_textsize(psy_ui_button_base(self), text);
		tm = psy_ui_component_textmetric(&self->component);
		if (self->enabled == FALSE) {
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00777777));
		} else {
			psy_ui_settextcolour(g, psy_ui_component_colour(&self->component));
		}
		psy_ui_textoutrectangle(g,
			psy_ui_button_center(self, psy_ui_realpoint_make(ident, 0.0),
				psy_ui_realsize_make(psy_ui_value_px(&textsize.width, tm),
				tm->tmHeight)),
			psy_ui_ETO_CLIPPED, r, text, strlen(text));
	}
	
}

psy_ui_RealPoint psy_ui_button_center(psy_ui_Button* self,
	psy_ui_RealPoint center, psy_ui_RealSize itemsize)
{
	psy_ui_RealPoint rv;
	psy_ui_RealSize sizepx;
	
	sizepx = psy_ui_component_sizepx(psy_ui_button_base(self));
	rv = center;
	if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_HORIZONTAL) ==
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL) {
		rv.x = center.x + (sizepx.width - itemsize.width - center.x) / 2;
	}
	if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_VERTICAL) ==
		psy_ui_ALIGNMENT_CENTER_VERTICAL) {
		rv.y = (sizepx.height - itemsize.height) / 2;
	}
	return rv;
}

void psy_ui_button_setcharnumber(psy_ui_Button* self, double number)
{
	self->charnumber = number;
}

void psy_ui_button_setlinespacing(psy_ui_Button* self, double spacing)
{
	self->linespacing = spacing;
}

void onpreferredsize(psy_ui_Button* self, psy_ui_Size* limit, psy_ui_Size* rv)
{		
	const psy_ui_TextMetric* tm;
	psy_ui_Size size;
	char* text;

	if (self->translate && self->translation) {
		text = self->translation;
	} else {
		text = self->text;
	}		
	tm = psy_ui_component_textmetric(psy_ui_button_base(self));
	if (self->charnumber == 0) {
		if (text) {
			size = psy_ui_component_textsize(psy_ui_button_base(self), text);
		} else {
			size = psy_ui_size_zero();
		}
		if (self->icon) {
			if (self->icon == psy_ui_ICON_LESS || self->icon == psy_ui_ICON_MORE) {				
				if (text) {
					psy_ui_Size textsize;

					textsize = psy_ui_size_makeem(2.0, 1.0);
					psy_ui_value_add(&size.width, &textsize.width, tm);
				} else {
					size.width = psy_ui_value_makeew(1.0);
				}
			} else {
				size.width = psy_ui_value_makeew(2.0);
			}
		}
		if (!psy_ui_bitmap_empty(&self->bitmapicon)) {
			psy_ui_RealSize srcbpmsize;			
			const psy_ui_TextMetric* tm;
			double ratio;

			tm = psy_ui_component_textmetric(psy_ui_button_base(self));
			srcbpmsize = psy_ui_bitmap_size(&self->bitmapicon);
			ratio = (tm->tmAscent - tm->tmDescent) / srcbpmsize.height;
			if (fabs(ratio - 1.0) < 0.15) {
				ratio = 1.0;
			}			
			rv->width = psy_ui_add_values(
				psy_ui_value_makepx(srcbpmsize.width * ratio + tm->tmAveCharWidth * self->bitmapident), size.width, tm);
			rv->height = psy_ui_value_makeeh(self->linespacing);
			return;
		}
		rv->width = psy_ui_value_makepx(psy_ui_value_px(&size.width, tm) + 4);
	} else {
		rv->width = psy_ui_value_makeew(self->charnumber);
	}
	rv->height = psy_ui_value_makeeh(self->linespacing);
}

void onmousedown(psy_ui_Button* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_capture(psy_ui_button_base(self));
}

void onmouseup(psy_ui_Button* self, psy_ui_MouseEvent* ev)
{		
	psy_ui_component_releasecapture(psy_ui_button_base(self));
	self->buttonstate = ev->button != 0;
	if (self->enabled && (ev->button == 1) || self->allowrightclick) {
		psy_ui_RealRectangle client_position;
		psy_ui_RealSize size;
		
		size = psy_ui_component_sizepx(psy_ui_button_base(self));
		client_position = psy_ui_realrectangle_make(
			psy_ui_realpoint_zero(), size);
		if (psy_ui_realrectangle_intersect(&client_position, ev->pt)) {				
			self->shiftstate = ev->shift;
			self->ctrlstate = ev->ctrl;
			psy_signal_emit(&self->signal_clicked, self, 0);
		}
		psy_ui_mouseevent_stoppropagation(ev);
	}	
}

void onmouseenter(psy_ui_Button* self)
{
	if (self->enabled) {
		self->hover = 1;
		button_updatestyles(self);
		psy_ui_component_invalidate(psy_ui_button_base(self));
	}
}

void onmouseleave(psy_ui_Button* self)
{		
	self->hover = 0;
	if (self->enabled) {		
		button_updatestyles(self);
		psy_ui_component_invalidate(psy_ui_button_base(self));
	}	
}

void psy_ui_button_settext(psy_ui_Button* self, const char* text)
{	
	assert(self);

	psy_strreset(&self->text, text);
	if (self->translate) {
		psy_strreset(&self->translation, psy_ui_translate(text));
	}
	psy_ui_component_invalidate(psy_ui_button_base(self));
}

void psy_ui_button_seticon(psy_ui_Button* self, psy_ui_ButtonIcon icon)
{
	self->icon = icon;
	psy_ui_component_invalidate(psy_ui_button_base(self));
}

void psy_ui_button_highlight(psy_ui_Button* self)
{
	if (!psy_ui_button_highlighted(self)) {				
		self->highlight = TRUE;
		button_updatestyles(self);
		psy_ui_component_invalidate(psy_ui_button_base(self));
	}
}

void psy_ui_button_disablehighlight(psy_ui_Button* self)
{
	if (psy_ui_button_highlighted(self)) {
		self->highlight = FALSE;
		button_updatestyles(self);		
		psy_ui_component_invalidate(psy_ui_button_base(self));
	}
}

bool psy_ui_button_highlighted(const psy_ui_Button* self)
{
	return self->highlight != FALSE;
}

void psy_ui_button_settextcolour(psy_ui_Button* self, psy_ui_Colour colour)
{	
	if (self->component.style.currstyle->colour.mode.set != colour.mode.set ||
			self->component.style.currstyle->colour.mode.inherited != colour.mode.inherited ||
			self->component.style.currstyle->colour.value != colour.value) {
		psy_ui_colour_set(&self->component.style.currstyle->colour, colour);
		psy_ui_component_invalidate(&self->component);
	}
}

void psy_ui_button_settextalignment(psy_ui_Button* self,
	psy_ui_Alignment alignment)
{
	self->textalignment = alignment;
}

void psy_ui_button_preventtranslation(psy_ui_Button* self)
{
	self->translate = FALSE;
	if (self->translation) {
		free(self->translation);
		self->translation = NULL;
	}
}

void enableinput(psy_ui_Button* self)
{
	if (!self->enabled) {
		self->enabled = TRUE;
		psy_ui_component_invalidate(psy_ui_button_base(self));
	}
}

void preventinput(psy_ui_Button* self)
{
	if (self->enabled) {
		self->enabled = FALSE;
		psy_ui_component_invalidate(psy_ui_button_base(self));
	}
}

void button_onkeydown(psy_ui_Button* self, psy_ui_KeyEvent* ev)
{
	if (self->enabled && ev->keycode == psy_ui_KEY_RETURN) {		
		psy_signal_emit(&self->signal_clicked, self, 0);
		psy_ui_keyevent_stoppropagation(ev);
	}
}

void button_updatestyles(psy_ui_Button* self)
{		
	if (self->highlight) {		
		self->component.style.currstyle = &self->component.style.select;
	} else if (self->hover) {
		self->component.style.currstyle = &self->component.style.hover;
	} else {
		self->component.style.currstyle = &self->component.style.style;
	}		
}
