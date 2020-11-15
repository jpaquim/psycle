// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uibutton.h"
#include <string.h>
#include <stdlib.h>

#include "../../detail/trace.h"
#include "../../detail/portable.h"

static unsigned int arrowcolor = 0x00777777;
static unsigned int arrowhighlightcolor = 0x00FFFFFF;

static void ondestroy(psy_ui_Button*, psy_ui_Component* sender);
static void ondraw(psy_ui_Button*, psy_ui_Graphics*);
static void drawicon(psy_ui_Button*, psy_ui_Graphics*);
static void drawarrow(psy_ui_Button*, psy_ui_IntPoint* arrow, psy_ui_Graphics*);
static void makearrow(psy_ui_IntPoint*, psy_ui_ButtonIcon icon, int x, int y);
static void onmousedown(psy_ui_Button*, psy_ui_MouseEvent*);
static void onmouseenter(psy_ui_Button*);
static void onmouseleave(psy_ui_Button*);
static void onpreferredsize(psy_ui_Button*, psy_ui_Size* limit, psy_ui_Size* rv);
static void enableinput(psy_ui_Button*);
static void preventinput(psy_ui_Button*);
static void button_onkeydown(psy_ui_Button*, psy_ui_KeyEvent*);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_Button* self)
{
	if (!vtable_initialized) {
		vtable = *(psy_ui_button_base(self)->vtable);
		vtable.enableinput = (psy_ui_fp_component_enableinput)enableinput;
		vtable.preventinput = (psy_ui_fp_component_preventinput)preventinput;
		vtable.ondraw = (psy_ui_fp_ondraw)ondraw;
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)onpreferredsize;
		vtable.onmousedown = (psy_ui_fp_onmousedown)onmousedown;
		vtable.onmouseenter = (psy_ui_fp_onmouseenter)onmouseenter;
		vtable.onmouseleave = (psy_ui_fp_onmouseleave)onmouseleave;		
		vtable.onkeydown = (psy_ui_fp_onkeydown)button_onkeydown;
		vtable_initialized = 1;
	}
}

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
	self->textalignment = psy_ui_ALIGNMENT_CENTER_VERTICAL |
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL;	
	self->enabled = TRUE;
	self->text = strdup("");
	self->textcolor = psy_ui_color_make(0x00CACACA);
	psy_signal_init(&self->signal_clicked);	
	psy_signal_connect(&psy_ui_button_base(self)->signal_destroy, self,
		ondestroy);
}	

void ondestroy(psy_ui_Button* self, psy_ui_Component* sender)
{	
	free(self->text);
	self->text = 0;
	psy_signal_dispose(&self->signal_clicked);	
}

void ondraw(psy_ui_Button* self, psy_ui_Graphics* g)
{
	psy_ui_Size size;
	psy_ui_Size textsize;
	psy_ui_TextMetric tm;
	psy_ui_Rectangle r;
	int centerx = 0;
	int centery = 0;

	size = psy_ui_component_size(psy_ui_button_base(self));
	textsize = psy_ui_component_textsize(psy_ui_button_base(self), self->text);
	tm = psy_ui_component_textmetric(&self->component);   
	psy_ui_setrectangle(&r, 0, 0, psy_ui_value_px(&size.width, &tm),
		psy_ui_value_px(&size.height, &tm));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	if (self->enabled == FALSE) {
		psy_ui_settextcolor(g, psy_ui_color_make(0x00777777));
	} else if (self->hover || psy_ui_component_hasfocus(&self->component)) {
		psy_ui_settextcolor(g, psy_ui_color_make(0x00FFFFFF));
	} else if (psy_ui_button_highlighted(self)) {
		psy_ui_settextcolor(g, psy_ui_color_make(0x00FFFFFF));
	} else {
		psy_ui_settextcolor(g, self->textcolor);
	}
	if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_HORIZONTAL) ==
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL) {
		centerx = (psy_ui_value_px(&size.width, &tm) - psy_ui_value_px(&textsize.width, &tm)) / 2;
	}
	if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_VERTICAL) ==
		psy_ui_ALIGNMENT_CENTER_VERTICAL) {
		centery = (psy_ui_value_px(&size.height, &tm) - psy_ui_value_px(&textsize.height, &tm)) / 2;
	}   
	psy_ui_textoutrectangle(g, 
		centerx,
		centery,
		psy_ui_ETO_CLIPPED,
		r,
		self->text,
		strlen(self->text));
	if (self->icon != psy_ui_ICON_NONE) {
		drawicon(self, g);
	}
}

void drawicon(psy_ui_Button* self, psy_ui_Graphics* g)
{
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	psy_ui_IntPoint arrow[4];
	int centerx = 4;
	int centery = 0;
	
	size = psy_ui_component_size(psy_ui_button_base(self));
	tm = psy_ui_component_textmetric(&self->component);
	if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_HORIZONTAL) ==
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL) {
		centerx = (psy_ui_value_px(&size.width, &tm) - 4) / 2;
	}
	if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_VERTICAL) ==
		psy_ui_ALIGNMENT_CENTER_VERTICAL) {
		centery = (psy_ui_value_px(&size.height, &tm) - 8) / 2;
	}
	if (self->icon == psy_ui_ICON_LESSLESS) {
		makearrow(arrow,
			psy_ui_ICON_LESS,
			centerx - 4,
			centery);
		drawarrow(self, arrow, g);
		makearrow(arrow,
			psy_ui_ICON_LESS,
			centerx + 4,
			centery);	
		drawarrow(self, arrow, g);
	} else
	if (self->icon == psy_ui_ICON_MOREMORE) {
		makearrow(arrow,
			psy_ui_ICON_MORE,
			centerx - 6,
			centery);
		drawarrow(self, arrow, g);
		makearrow(arrow,
			psy_ui_ICON_MORE,
			centerx + 2,
			centery);	
		drawarrow(self, arrow, g);
	} else {
		makearrow(arrow,
			self->icon, 
			centerx - 2,
			centery);
		drawarrow(self, arrow, g);
	}
}

void drawarrow(psy_ui_Button* self, psy_ui_IntPoint* arrow, psy_ui_Graphics* g)
{
	if (self->hover == 1) {
		psy_ui_drawsolidpolygon(g, arrow, 4, arrowhighlightcolor,
			arrowhighlightcolor);
	} else {		
		psy_ui_drawsolidpolygon(g, arrow, 4, arrowcolor, arrowcolor);
	}		
}

void makearrow(psy_ui_IntPoint* arrow, psy_ui_ButtonIcon icon, int x, int y)
{
	switch (icon) {
		case psy_ui_ICON_LESS:		
			arrow[0].x = 4 + x;
			arrow[0].y = 0 + y;
			arrow[1].x = 4 + x;
			arrow[1].y = 8 + y;
			arrow[2].x = 0 + x;
			arrow[2].y = 4 + y;
			arrow[3] = arrow[0];
			break;
		case psy_ui_ICON_MORE:
			arrow[0].x = 0 + x;
			arrow[0].y = 0 + y;
			arrow[1].x = 0 + x;
			arrow[1].y = 8 + y;
			arrow[2].x = 4 + x;
			arrow[2].y = 4 + y;
			arrow[3] = arrow[0];
			break;
		case psy_ui_ICON_UP:
			arrow[0].x = 0 + x;
			arrow[0].y = 4 + y;
			arrow[1].x = 8 + x;
			arrow[1].y = 4 + y;
			arrow[2].x = 4 + x;
			arrow[2].y = 0 + y;
			arrow[3] = arrow[0];
			break;
		case psy_ui_ICON_DOWN:
			arrow[0].x = 0 + x;
			arrow[0].y = 0 + y;
			arrow[1].x = 8 + x;
			arrow[1].y = 0 + y;
			arrow[2].x = 4 + x;
			arrow[2].y = 4 + y;
			arrow[3] = arrow[0];
			break;
		default:
		break;
	}
}

void psy_ui_button_setcharnumber(psy_ui_Button* self, int number)
{
	self->charnumber = number;
}

void psy_ui_button_setlinespacing(psy_ui_Button* self, double spacing)
{
	self->linespacing = spacing;
}

void onpreferredsize(psy_ui_Button* self, psy_ui_Size* limit, psy_ui_Size* rv)
{		
	psy_ui_TextMetric tm;	
	psy_ui_Size size;
		
	tm = psy_ui_component_textmetric(psy_ui_button_base(self));
	if (self->charnumber == 0) {
		size = psy_ui_component_textsize(psy_ui_button_base(self), self->text);
		if (self->icon) {
			if (self->icon == psy_ui_ICON_LESS || self->icon == psy_ui_ICON_MORE) {
				psy_ui_Size textsize;

				textsize = psy_ui_component_textsize(psy_ui_button_base(self), "<");
				size.width = psy_ui_value_makepx(
					psy_ui_value_px(&size.width, &tm) + 
					psy_ui_value_px(&textsize.width, &tm));
			} else {
				size.width = psy_ui_component_textsize(psy_ui_button_base(self), "<<").width;
			}
		}		
		rv->width = psy_ui_value_makepx(psy_ui_value_px(&size.width, &tm) + 4);
	} else {
		rv->width = psy_ui_value_makeew(self->charnumber);
	}
	rv->height = psy_ui_value_makepx((int)(tm.tmHeight * self->linespacing));
}

void onmousedown(psy_ui_Button* self, psy_ui_MouseEvent* ev)
{
	if (self->enabled) {
		psy_signal_emit(&self->signal_clicked, self, 0);
	}
}

void onmouseenter(psy_ui_Button* self)
{
	if (self->enabled) {
		self->hover = 1;
		psy_ui_component_invalidate(psy_ui_button_base(self));
	}
}

void onmouseleave(psy_ui_Button* self)
{		
	if (self->enabled) {		
		psy_ui_component_invalidate(psy_ui_button_base(self));
	}
	self->hover = 0;
}

void psy_ui_button_settext(psy_ui_Button* self, const char* text)
{	
	psy_strreset(&self->text, text);	
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
		psy_ui_component_invalidate(psy_ui_button_base(self));
	}
}

void psy_ui_button_disablehighlight(psy_ui_Button* self)
{
	if (psy_ui_button_highlighted(self)) {
		self->highlight = FALSE;
		psy_ui_component_invalidate(psy_ui_button_base(self));
	}
}

bool psy_ui_button_highlighted(psy_ui_Button* self)
{
	return self->highlight != FALSE;
}

void psy_ui_button_settextcolor(psy_ui_Button* self, psy_ui_Color color)
{
	if (self->textcolor.value != color.value) {
		self->textcolor = color;
		psy_ui_component_invalidate(&self->component);
	}
}

void psy_ui_button_settextalignment(psy_ui_Button* self,
	psy_ui_Alignment alignment)
{
	self->textalignment = alignment;
}

void enableinput(psy_ui_Button* self)
{
	if (self->enabled == FALSE) {
		self->enabled = TRUE;
		psy_ui_component_invalidate(psy_ui_button_base(self));
	}
}

void preventinput(psy_ui_Button* self)
{
	if (self->enabled == TRUE) {
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
