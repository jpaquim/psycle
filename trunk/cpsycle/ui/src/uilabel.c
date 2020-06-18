// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uilabel.h"

#include <stdlib.h>
#include <string.h>

#include "../../detail/portable.h"

static void psy_ui_label_ondestroy(psy_ui_Label*, psy_ui_Component* sender);
static void psy_ui_label_ondraw(psy_ui_Label*, psy_ui_Graphics*);
static void psy_ui_label_onpreferredsize(psy_ui_Label*, psy_ui_Size* limit, psy_ui_Size* rv);

static psy_ui_ComponentVtable psy_ui_label_vtable;
static int psy_ui_label_vtable_initialized = 0;

static void psy_ui_label_vtable_init(psy_ui_Label* self)
{
	if (!psy_ui_label_vtable_initialized) {
		psy_ui_label_vtable = *(self->component.vtable);
		psy_ui_label_vtable.ondraw = (psy_ui_fp_ondraw)psy_ui_label_ondraw;
		psy_ui_label_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)psy_ui_label_onpreferredsize;
		psy_ui_label_vtable_initialized = 1;
	}
}

void psy_ui_label_init(psy_ui_Label* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);	
	psy_ui_label_vtable_init(self);
	self->component.vtable = &psy_ui_label_vtable;
	self->charnumber = 0;
	self->textalignment = psy_ui_ALIGNMENT_CENTER_VERTICAL |
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL;
	self->text = strdup("");
	psy_signal_connect(&self->component.signal_destroy, self,
		psy_ui_label_ondestroy);
}

void psy_ui_label_ondestroy(psy_ui_Label* self, psy_ui_Component* sender)
{
	free(self->text);
}

void psy_ui_label_settext(psy_ui_Label* self, const char* text)
{
	free(self->text);
	self->text = strdup(text ? text : "");
	psy_ui_component_invalidate(psy_ui_label_base(self));
}

void psy_ui_label_text(psy_ui_Label* self, char* text)
{
	psy_snprintf(self->text, 256, "%s", text);
}

void psy_ui_label_onpreferredsize(psy_ui_Label* self, psy_ui_Size* limit, psy_ui_Size* rv)
{
	if (rv) {
		psy_ui_TextMetric tm;		

		tm = psy_ui_component_textmetric(psy_ui_label_base(self));
		if (self->charnumber == 0) {
			psy_ui_Size size;
			
			size = psy_ui_component_textsize(psy_ui_label_base(self), self->text);
			rv->width = psy_ui_value_makepx(psy_ui_value_px(&size.width, &tm) + 2 +
				psy_ui_margin_width_px(&psy_ui_label_base(self)->spacing, &tm));
		} else {
			rv->width = psy_ui_value_makepx(tm.tmAveCharWidth * self->charnumber);
		}
		rv->height = psy_ui_value_makepx(tm.tmHeight +
			psy_ui_margin_height_px(&psy_ui_label_base(self)->spacing, &tm));
	}
}

void psy_ui_label_ondraw(psy_ui_Label* self, psy_ui_Graphics* g)
{
	psy_ui_Size size;
	psy_ui_Size textsize;
	psy_ui_TextMetric tm;
	psy_ui_Rectangle r;
	int centerx = 0;
	int centery = 0;

	size = psy_ui_component_size(psy_ui_label_base(self));
	textsize = psy_ui_component_textsize(psy_ui_label_base(self), self->text);
	tm = psy_ui_component_textmetric(&self->component);
	psy_ui_setrectangle(&r, 0, 0, psy_ui_value_px(&size.width, &tm),
		psy_ui_value_px(&size.height, &tm));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolor(g, psy_ui_component_color(&self->component));
	psy_ui_textoutrectangle(g,
		centerx,
		centery,
		psy_ui_ETO_CLIPPED,
		r,
		self->text,
		strlen(self->text));
}

void psy_ui_label_setcharnumber(psy_ui_Label* self, int number)
{
	self->charnumber = number;
}

void psy_ui_label_settextalignment(psy_ui_Label* self, psy_ui_Alignment alignment)
{
	self->textalignment = alignment;
}
