// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uibutton.h"
#include <string.h>
#include <stdlib.h>

static unsigned int arrowcolor = 0x00777777;
static unsigned int arrowhighlightcolor = 0x00FFFFFF;

static void ondestroy(psy_ui_Button*, psy_ui_Component* sender);
static void ondraw(psy_ui_Button*, psy_ui_Graphics*);
static void drawicon(psy_ui_Button*, psy_ui_Graphics*);
static void drawarrow(psy_ui_Button*, psy_ui_Point* arrow, psy_ui_Graphics*);
static void makearrow(psy_ui_Point*, psy_ui_ButtonIcon icon, int x, int y);
static void onmousedown(psy_ui_Button*, psy_ui_MouseEvent*);
static void onmouseenter(psy_ui_Button*);
static void onmouseleave(psy_ui_Button*);
static void onpreferredsize(psy_ui_Button*, psy_ui_Size* limit, psy_ui_Size* size);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_Button* self)
{
	if (!vtable_initialized) {
		vtable = *(psy_ui_button_base(self)->vtable);
		vtable.ondraw = (psy_ui_fp_ondraw) ondraw;
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize) onpreferredsize;
		vtable.onmousedown = (psy_ui_fp_onmousedown) onmousedown;
		vtable.onmouseenter = (psy_ui_fp_onmouseenter) onmouseenter;
		vtable.onmouseleave = (psy_ui_fp_onmouseleave) onmouseleave;
		vtable_initialized = 1;
	}
}

void psy_ui_button_init(psy_ui_Button* self, psy_ui_Component* parent)
{	
	psy_ui_component_init(psy_ui_button_base(self), parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	self->hover = 0;
	self->highlight = 0;
	self->icon = psy_ui_ICON_NONE;
	self->charnumber = 0;
	self->textalignment = psy_ui_ALIGNMENT_CENTER_VERTICAL |
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL;	
	self->text = _strdup("");
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
	psy_ui_Rectangle r;
	int centerx = 0;
	int centery = 0;
		
	size = psy_ui_component_size(psy_ui_button_base(self));
	textsize = psy_ui_component_textsize(psy_ui_button_base(self), self->text);
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	if (self->hover) {
		psy_ui_settextcolor(g, 0x00FFFFFF);
	} else 
	if (self->highlight) {
		psy_ui_settextcolor(g, 0x00FFFFFF);
	} else
	{
		psy_ui_settextcolor(g, 0x00CACACA);
	}
	if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_HORIZONTAL) ==
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL) {
		centerx = (size.width - textsize.width) / 2;		
	}
	if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_VERTICAL) ==
		psy_ui_ALIGNMENT_CENTER_VERTICAL) {
		centery = (size.height - textsize.height) / 2;
	}	
	psy_ui_textoutrectangle(g, 
		centerx,
		centery,
		ETO_CLIPPED,
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
	psy_ui_Point arrow[4];
	int centerx = 4;
	int centery = 0;
	
	size = psy_ui_component_size(psy_ui_button_base(self));
	if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_HORIZONTAL) ==
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL) {
		centerx = (size.width - 4) / 2;		
	}
	if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_VERTICAL) ==
		psy_ui_ALIGNMENT_CENTER_VERTICAL) {
		centery = (size.height - 8) / 2;
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

void drawarrow(psy_ui_Button* self, psy_ui_Point* arrow, psy_ui_Graphics* g)
{
	if (self->hover == 1) {
		psy_ui_drawsolidpolygon(g, arrow, 4, arrowhighlightcolor,
			arrowhighlightcolor);
	} else {		
		psy_ui_drawsolidpolygon(g, arrow, 4, arrowcolor, arrowcolor);
	}		
}

void makearrow(psy_ui_Point* arrow, psy_ui_ButtonIcon icon, int x, int y)
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
		default:
		break;
	}
}

void psy_ui_button_setcharnumber(psy_ui_Button* self, int number)
{
	self->charnumber = number;
}

void onpreferredsize(psy_ui_Button* self, psy_ui_Size* limit, psy_ui_Size* rv)
{		
	psy_ui_TextMetric tm;	
	psy_ui_Size size;		
		
	tm = psy_ui_component_textmetric(psy_ui_button_base(self));
	if (self->charnumber == 0) {
		if (self->icon) {
			if (self->icon == psy_ui_ICON_LESS || self->icon == psy_ui_ICON_MORE) {
				size = psy_ui_component_textsize(psy_ui_button_base(self), "<");
			} else {
				size = psy_ui_component_textsize(psy_ui_button_base(self), "<<");
			}
		} else {
			size = psy_ui_component_textsize(psy_ui_button_base(self), self->text);	
		}
		rv->width = size.width + 4;				
	} else {
		rv->width = tm.tmAveCharWidth * self->charnumber;
	}
	rv->height = tm.tmHeight;	
}

void onmousedown(psy_ui_Button* self, psy_ui_MouseEvent* ev)
{
	psy_signal_emit(&self->signal_clicked, self, 0);
}

void onmouseenter(psy_ui_Button* self)
{
	self->hover = 1;
	psy_ui_component_invalidate(psy_ui_button_base(self));
}

void onmouseleave(psy_ui_Button* self)
{		
	self->hover = 0;
	psy_ui_component_invalidate(psy_ui_button_base(self));
}

void psy_ui_button_settext(psy_ui_Button* self, const char* text)
{	
	free(self->text);
	self->text = _strdup(text);
	psy_ui_component_invalidate(psy_ui_button_base(self));
}

void psy_ui_button_seticon(psy_ui_Button* self, psy_ui_ButtonIcon icon)
{
	self->icon = icon;
}

void psy_ui_button_highlight(psy_ui_Button* self)
{
	if (!self->highlight) {
		self->highlight = 1;		
		psy_ui_component_invalidate(psy_ui_button_base(self));
	}
}

void psy_ui_button_disablehighlight(psy_ui_Button* self)
{
	if (self->highlight) {
		self->highlight = 0;
		psy_ui_component_invalidate(psy_ui_button_base(self));
	}
}

void psy_ui_button_settextalignment(psy_ui_Button* self,
	psy_ui_Alignment alignment)
{
	self->textalignment = alignment;
}

psy_ui_Component* psy_ui_button_base(psy_ui_Button* self)
{
	return &self->component;
}

